/* -*- mode: c++; c-basic-offset:4 -*-
    smartcard/readerstatus.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "readerstatus.h"

#include <utils/gnupg-helper.h>
#include <utils/stl_util.h>
#include <utils/kdsignalblocker.h>
#include <utils/filesystemwatcher.h>

#include <gpgme++/context.h>
#include <gpgme++/assuanresult.h>
#include <gpgme++/defaultassuantransaction.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QTimer>
#include <QPointer>
#include <QDebug>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/static_assert.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <set>
#include <list>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstdlib>

using namespace Kleo;
using namespace Kleo::SmartCard;
using namespace GpgME;
using namespace boost;

static const unsigned int MAX_RETRIES = 3;
static const unsigned int RETRY_WAIT = 2; // seconds
static const unsigned int CHECK_INTERVAL = 2000; // msecs

static ReaderStatus * self = 0;

struct CardInfo {
    CardInfo()
        : fileName(),
          status( ReaderStatus::NoCard ),
          appType( ReaderStatus::UnknownApplication ),
          appVersion( -1 )
    {

    }
    CardInfo( const QString & fn, ReaderStatus::Status s )
        : fileName( fn ),
          status( s ),
          appType( ReaderStatus::UnknownApplication ),
          appVersion( -1 )
    {

    }

    QString fileName;
    ReaderStatus::Status status;
    std::string serialNumber;
    ReaderStatus::AppType appType;
    int appVersion;
    std::vector<ReaderStatus::PinState> pinStates;
};

static const char * flags[] = {
    "NOCARD",
    "PRESENT",
    "ACTIVE",
    "USABLE",
};
BOOST_STATIC_ASSERT(( sizeof flags/sizeof *flags == ReaderStatus::_NumScdStates ));

static const char * prettyFlags[] = {
    "NoCard",
    "CardPresent",
    "CardActive",
    "CardUsable",
    "CardCanLearnKeys",
    "CardHasNullPin",
};
BOOST_STATIC_ASSERT(( sizeof prettyFlags/sizeof *prettyFlags == ReaderStatus::NumStates ));

static ReaderStatus::Status read_status( const QString & fileName ) {
    QFile file( fileName );
    if ( !file.exists() ) {
        qDebug() << "read_status: file" << fileName << "does not exist";
        return ReaderStatus::NoCard;
    }
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qDebug() << "read_status: failed to open" << fileName << ':' << file.errorString();
        return ReaderStatus::NoCard;
    }
    const QByteArray contents = file.readAll().trimmed();
    const char ** it = std::find( begin( flags ), end( flags ), contents );
    if ( it == end( flags ) ) {
        qDebug() << "read_status: not found";
        return ReaderStatus::NoCard;
    } else {
        return static_cast<ReaderStatus::Status>( it - begin( flags ) );
    }
}

static unsigned int parseFileName( const QString & fileName, bool * ok ) {
    QRegExp rx( QLatin1String( "reader_(\\d+)\\.status" ) );
    if ( ok )
        *ok = false;
    if ( rx.exactMatch( QFileInfo( fileName ).fileName() ) )
        return rx.cap(1).toUInt( ok, 10 );
    return 0;
}

namespace {
    template <typename T_Target, typename T_Source>
    std::auto_ptr<T_Target> dynamic_pointer_cast( std::auto_ptr<T_Source> & in ) {
        if ( T_Target * const target = dynamic_cast<T_Target*>( in.get() ) ) {
            in.release();
            return std::auto_ptr<T_Target>( target );
        } else {
            return std::auto_ptr<T_Target>();
        }
    }

    template <typename T>
    const T & _trace__impl( const T & t, const char * msg ) {
        qDebug() << msg << t;
        return t;
    }
#define TRACE( x ) _trace__impl( x, #x )
}

static QDebug operator<<( QDebug s, const std::vector< std::pair<std::string,std::string> > & v ) {
    typedef std::pair<std::string,std::string> pair;
    s << '(';
    Q_FOREACH( const pair & p, v )
        s << "status(" << QString::fromStdString( p.first ) << ") =" << QString::fromStdString( p.second ) << endl;
    return s << ')';
}

static const char * app_types[] = {
    "_", // will hopefully never be used as an app-type :)
    "openpgp",
    "nks",
    "p15",
    "dinsig",
    "geldkarte",
};
BOOST_STATIC_ASSERT(( sizeof app_types / sizeof *app_types == ReaderStatus::NumAppTypes ));
    

static ReaderStatus::AppType parse_app_type( const std::string & s ) {
    qDebug() << "parse_app_type(" << s.c_str() << ")";
    const char ** it = std::find( begin( app_types ), end( app_types ), to_lower_copy( s ) );
    if ( it == end( app_types ) )
        return TRACE( ReaderStatus::UnknownApplication );
    return TRACE( static_cast<ReaderStatus::AppType>( it - begin( app_types ) ) );
}

static int parse_app_version( const std::string & s ) {
    return std::atoi( s.c_str() );
}

static ReaderStatus::PinState parse_pin_state( const std::string & s ) {
    switch ( int i = std::atoi( s.c_str() ) ) {
    case -4: return ReaderStatus::NullPin;
    case -3: return ReaderStatus::PinBlocked;
    case -2: return ReaderStatus::NoPin;
    case -1: return ReaderStatus::UnknownPinState;
    default:
        if ( i < 0 )
            return ReaderStatus::UnknownPinState;
        else
            return ReaderStatus::PinOk;
    }
}

static void ReaderStatusThread_sleep( unsigned int secs );

static std::auto_ptr<DefaultAssuanTransaction> gpgagent_transact( shared_ptr<Context> & gpgAgent, const char * command, Error & err ) {
    for ( unsigned int i = 0 ; i < MAX_RETRIES ; ++i ) {
        if ( i > 0 )
            qDebug() << "gpgagent_transact(" << command << "), try #" << i;
        else
            qDebug() << "gpgagent_transact(" << command << ")";
        const AssuanResult res = gpgAgent->assuanTransact( command );
        err = res.error();
        if ( !err.code() )
            err = res.assuanError();
        if ( err.code() ) {
            qDebug() << "gpgagent_transact(" << command << "):" << QString::fromLocal8Bit( err.asString() );
            if ( err.code() >= GPG_ERR_ASS_GENERAL && err.code() <= GPG_ERR_ASS_UNKNOWN_INQUIRE ) {
                qDebug() << "Assuan problem, try re-creating the assuan context in " << RETRY_WAIT << "sec.";
                gpgAgent.reset();
                qDebug() << "waiting for gpg-agent ...zZZ";
                ReaderStatusThread_sleep( RETRY_WAIT );
                qDebug() << "returning from waiting for gpg-agent ...oOO";
                std::auto_ptr<Context> c = Context::createForEngine( AssuanEngine );
                gpgAgent = c;
                continue;
            }
            return std::auto_ptr<DefaultAssuanTransaction>();
        }
        std::auto_ptr<AssuanTransaction> t = gpgAgent->takeLastAssuanTransaction();
        return dynamic_pointer_cast<DefaultAssuanTransaction>( t );
    }
    qDebug() << "Max. retries reached, giving up.";
    return std::auto_ptr<DefaultAssuanTransaction>();
}

// returns const std::string so template deduction in boost::split works, and we don't need a temporary
static const std::string scd_getattr_status( shared_ptr<Context> & gpgAgent, const char * what, Error & err ) {
    std::string cmd = "SCD GETATTR ";
    cmd += what;
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, cmd.c_str(), err );
    if ( t.get() ) {
        qDebug() << "scd_getattr_status(" << what << "): got" << t->statusLines();
        return t->firstStatusLine( what );
    } else {
        qDebug() << "scd_getattr_status(" << what << "): t == NULL";
        return std::string();
    }
}

static unsigned int parse_event_counter( const std::string & str ) {
    unsigned int result;
    if ( sscanf( str.c_str(), "%*u %*u %u ", &result ) == 1 )
        return result;
    return 0U;
}

static unsigned int get_event_counter( shared_ptr<Context> & gpgAgent ) {
    Error err;
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, "GETEVENTCOUNTER", err );
    if ( err.code() )
        qDebug() << "get_event_counter(): got error" << err.asString();
    if ( t.get() ) {
        qDebug() << "get_event_counter(): got" << t->statusLines();
        return parse_event_counter( t->firstStatusLine( "EVENTCOUNTER" ) );
    } else {
        qDebug() << "scd_getattr_status(): t == NULL";
        return 0U;
    }
}

// returns const std::string so template deduction in boost::split works, and we don't need a temporary
static const std::string gpgagent_data( shared_ptr<Context> & gpgAgent, const char * what, Error & err ) {
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, what, err );
    if ( t.get() )
        return t->data();
    else
        return std::string();
}

static std::string parse_keypairinfo( const std::string & kpi ) {
    static const char hexchars[] = "0123456789abcdefABCDEF";
    return '&' + kpi.substr( 0, kpi.find_first_not_of( hexchars ) );
}

static bool parse_keypairinfo_and_lookup_key( Context * ctx, const std::string & kpi ) {
    if ( !ctx )
        return false;
    const std::string pattern = parse_keypairinfo( kpi );
    qDebug() << "parse_keypairinfo_and_lookup_key: pattern=" << pattern.c_str();
    if ( const Error err = ctx->startKeyListing( pattern.c_str() ) ) {
        qDebug() << "parse_keypairinfo_and_lookup_key: startKeyListing failed:" << err.asString();
        return false;
    }
    Error e;
    const Key key = ctx->nextKey( e );
    ctx->endKeyListing();
    qDebug() << "parse_keypairinfo_and_lookup_key: e=" << e.code() << "; key.isNull()" << key.isNull();
    return !e && !key.isNull();
}

static CardInfo get_more_detailed_status( const QString & fileName, unsigned int idx, shared_ptr<Context> & gpg_agent ) {
    qDebug() << "get_more_detailed_status(" << fileName << ',' << idx << ',' << gpg_agent.get() << ')';
    CardInfo ci( fileName, ReaderStatus::CardUsable );
    if ( idx != 0 || !gpg_agent )
        return ci;
    Error err;
    ci.serialNumber = gpgagent_data( gpg_agent, "SCD SERIALNO", err );
    if ( err.code() == GPG_ERR_CARD_NOT_PRESENT || err.code() == GPG_ERR_CARD_REMOVED ) {
        ci.status = ReaderStatus::NoCard;
        return ci;
    }
    ci.appType = parse_app_type( scd_getattr_status( gpg_agent, "APPTYPE", err ) );
    if ( err.code() )
        return ci;
    if ( ci.appType != ReaderStatus::NksApplication ) {
        qDebug() << "get_more_detailed_status: not a NetKey card, giving up";
        return ci;
    }
    ci.appVersion = parse_app_version( scd_getattr_status( gpg_agent, "NKS-VERSION", err ) );
    if ( err.code() )
        return ci;
    if ( ci.appVersion != 3 ) {
        qDebug() << "get_more_detailed_status: not a NetKey v3 card, giving up";
        return ci;
    }

    // the following only works for NKS v3...
    std::vector<std::string> chvStatus;
    chvStatus.reserve( 4 ); // expected number of fields
    split( chvStatus, scd_getattr_status( gpg_agent, "CHV-STATUS", err ), is_any_of( " \t" ), token_compress_on );
    if ( err.code() )
        return ci;
    std::transform( chvStatus.begin(), chvStatus.end(),
                    std::back_inserter( ci.pinStates ),
                    parse_pin_state );

    if ( kdtools::contains( ci.pinStates, ReaderStatus::NullPin ) ) {
        ci.status = ReaderStatus::CardHasNullPin;
        return ci;
    }

    // check for keys to learn:
    const std::auto_ptr<DefaultAssuanTransaction> result = gpgagent_transact( gpg_agent, "SCD LEARN --keypairinfo", err );
    if ( err.code() || !result.get() )
        return ci;
    const std::vector<std::string> keyPairInfos = result->statusLine( "KEYPAIRINFO" );
    if ( keyPairInfos.empty() )
        return ci;

    // check that any of the 
    const std::auto_ptr<Context> klc( Context::createForProtocol( CMS ) ); // what about OpenPGP?
    if ( !klc.get() )
        return ci;
    klc->setKeyListMode( Ephemeral );

    if ( kdtools::any( keyPairInfos, !bind( &parse_keypairinfo_and_lookup_key, klc.get(), _1 ) ) )
        ci.status = ReaderStatus::CardCanLearnKeys;

    qDebug() << "get_more_detailed_status: ci.status " << prettyFlags[ci.status];

    return ci;
}

static CardInfo get_card_info( const QString & fileName, unsigned int idx, shared_ptr<Context> & gpg_agent, const CardInfo & oldInfo, bool force ) {
    qDebug() << "get_card_info(" << fileName << ',' << idx << ',' << gpg_agent.get() << ',' << prettyFlags[oldInfo.status] << ',' << force << ')';
    const ReaderStatus::Status st = read_status( fileName );
    if ( force && gpg_agent ||
         ( st == ReaderStatus::CardUsable || st == ReaderStatus::CardPresent ) && st != oldInfo.status && gpg_agent )
        return get_more_detailed_status( fileName, idx, gpg_agent );
    else
        return CardInfo( fileName, st );
}

static std::vector<CardInfo> update_cardinfo( const QString & gnupgHomePath, shared_ptr<Context> & gpgAgent, const std::vector<CardInfo> & oldCardInfos, bool force ) {
    qDebug() << "<update_cardinfo>";
    const QDir gnupgHome( gnupgHomePath );
    if ( !gnupgHome.exists() )
        qWarning() << "update_cardinfo: gnupg home" << gnupgHomePath << "does not exist!";

    QStringList files = gnupgHome.entryList( QStringList( QLatin1String( "reader_*.status" ) ), QDir::Files, QDir::Name );
    if ( files.empty() )
        files.push_back( QLatin1String( "reader_0.status" ) ); // not always present on Windows, so fake it

    std::vector<CardInfo> result;
    result.reserve( files.size() );
    Q_FOREACH( const QString & file, files ) {
        bool ok = false;
        const unsigned int idx = parseFileName( file, &ok );
        if ( !ok ) {
            qDebug() << "filename" << file << ": cannot parse reader slot number";
            continue;
        }
        result.resize( idx );
        result.push_back( get_card_info( gnupgHome.absoluteFilePath( file ), idx, gpgAgent,
                                         idx < oldCardInfos.size() ? oldCardInfos[idx] : CardInfo(), force ) );
    }
    qDebug() << "</update_cardinfo>";
    return result;
}

static bool check_event_counter_changed( shared_ptr<Context> & gpg_agent, unsigned int & counter ) {
    const unsigned int oldCounter = counter;
    counter = get_event_counter( gpg_agent );
    if ( oldCounter != counter ) {
        qDebug() << "ReaderStatusThread[2nd]: events:" << oldCounter << "->" << counter ;
        return true;
    } else {
        return false;
    }
}

struct Transaction {
    QByteArray command;
    QPointer<QObject> receiver;
    const char * slot;
    GpgME::Error error;
};

static const Transaction checkTransaction  = { "__check__",  0, 0, Error() };
static const Transaction updateTransaction = { "__update__", 0, 0, Error() };
static const Transaction quitTransaction   = { "__quit__",   0, 0, Error() };

namespace {
    class ReaderStatusThread : public QThread {
        Q_OBJECT
    public:
        explicit ReaderStatusThread( QObject * parent=0 )
            : QThread( parent ),
              m_gnupgHomePath( Kleo::gnupgHomeDirectory() ),
              m_transactions( 1, updateTransaction ) // force initial scan
        {
            connect( this, SIGNAL(oneTransactionFinished()),
                     this, SLOT(slotOneTransactionFinished()) );
        }

        std::vector<CardInfo> cardInfos() const {
            const QMutexLocker locker( &m_mutex );
            return m_cardInfos;
        }

        ReaderStatus::Status cardStatus( unsigned int slot ) const {
            const QMutexLocker locker( &m_mutex );
            if ( slot < m_cardInfos.size() )
                return m_cardInfos[slot].status;
            else
                return ReaderStatus::NoCard;
        }

        void addTransaction( const Transaction & t ) {
            const QMutexLocker locker( &m_mutex );
            m_transactions.push_back( t );
            m_waitForTransactions.wakeOne();
        }

        // make QThread::sleep public
        using QThread::sleep;

    Q_SIGNALS:
        void anyCardHasNullPinChanged( bool );
        void anyCardCanLearnKeysChanged( bool );
        void cardStatusChanged( unsigned int, Kleo::SmartCard::ReaderStatus::Status );
        void oneTransactionFinished();

    public Q_SLOTS:
        void ping() {
            qDebug() << "ReaderStatusThread[GUI]::ping()";
            addTransaction( updateTransaction );
        }

        void stop() {
            const QMutexLocker locker( &m_mutex );
            m_transactions.push_front( quitTransaction );
            m_waitForTransactions.wakeOne();
        }

    private Q_SLOTS:
        void slotOneTransactionFinished() {
            std::list<Transaction> ft;
            KDAB_SYNCHRONIZED( m_mutex )
                ft.splice( ft.begin(), m_finishedTransactions );
            Q_FOREACH( const Transaction & t, ft )
                if ( t.receiver && t.slot && *t.slot )
                    QMetaObject::invokeMethod( t.receiver, t.slot, Qt::DirectConnection, Q_ARG( GpgME::Error, t.error ) );
        }

    private:
        /* reimp */ void run() {
            std::auto_ptr<Context> c = Context::createForEngine( AssuanEngine );
            shared_ptr<Context> gpgAgent( c );

            unsigned int eventCounter = -1;

            while ( true ) {

                QByteArray command;
                bool nullSlot;
                std::list<Transaction> item;
                std::vector<CardInfo> oldCardInfos;

                KDAB_SYNCHRONIZED( m_mutex ) {

                    while ( m_transactions.empty() ) {
                        // go to sleep waiting for more work:
                        qDebug( "ReaderStatusThread[2nd]: .zZZ" );
                        if ( !m_waitForTransactions.wait( &m_mutex, CHECK_INTERVAL ) )
                            m_transactions.push_front( checkTransaction );
                        qDebug( "ReaderStatusThread[2nd]: .oOO" );
                    }

                    // splice off the first transaction without
                    // copying, so we own it without really importing
                    // it into this thread (the QPointer isn't
                    // thread-safe):
                    item.splice( item.end(),
                                 m_transactions, m_transactions.begin() );
                    
                    // make local copies of the interesting stuff so
                    // we can release the mutex again:
                    command = item.front().command;
                    nullSlot = !item.front().slot;
                    oldCardInfos = m_cardInfos;
                }

                qDebug() << "ReaderStatusThread[2nd]: new iteration command=" << command << " ; nullSlot=" << nullSlot;

                // now, let's see what we got:

                if ( nullSlot && command == quitTransaction.command )
                    return; // quit

                if ( nullSlot && command == updateTransaction.command ||
                     nullSlot && command == checkTransaction.command ) {

                    if ( nullSlot && command == checkTransaction.command && !check_event_counter_changed( gpgAgent, eventCounter ) )
                        continue; // early out

                    std::vector<CardInfo> newCardInfos
                        = update_cardinfo( m_gnupgHomePath, gpgAgent, oldCardInfos, true );

                    newCardInfos.resize( std::max( newCardInfos.size(), oldCardInfos.size() ) );
                    oldCardInfos.resize( std::max( newCardInfos.size(), oldCardInfos.size() ) );

                    KDAB_SYNCHRONIZED( m_mutex )
                        m_cardInfos = newCardInfos;

                    std::vector<CardInfo>::const_iterator
                        nit = newCardInfos.begin(), nend = newCardInfos.end(),
                        oit = oldCardInfos.begin(), oend = oldCardInfos.end() ;

                    unsigned int idx = 0;
                    bool anyLC = false;
                    bool anyNP = false;
                    while ( nit != nend && oit != oend ) {
                        if ( nit->status != oit->status ) {
                            qDebug() << "ReaderStatusThread[2nd]: slot" << idx << ":" << prettyFlags[oit->status] << "->" << prettyFlags[nit->status];
                            emit cardStatusChanged( idx, nit->status );
                        }
                        if ( nit->status == ReaderStatus::CardCanLearnKeys )
                            anyLC = true;
                        if ( nit->status == ReaderStatus::CardHasNullPin )
                            anyNP = true;
                        ++nit;
                        ++oit;
                        ++idx;
                    }

                    emit anyCardHasNullPinChanged( anyNP );
                    emit anyCardCanLearnKeysChanged( anyLC );

                } else {

                    (void)gpgagent_transact( gpgAgent, command.constData(), item.front().error );

                    KDAB_SYNCHRONIZED( m_mutex )
                        // splice 'item' into m_finishedTransactions:
                        m_finishedTransactions.splice( m_finishedTransactions.end(), item );

                    emit oneTransactionFinished();

                }

                // update event counter in case anything above changed
                // it:
                eventCounter = get_event_counter( gpgAgent );
            }
        }

    private:
        mutable QMutex m_mutex;
        QWaitCondition m_waitForTransactions;
        const QString m_gnupgHomePath;
        // protected by m_mutex:
        std::vector<CardInfo> m_cardInfos;
        std::list<Transaction> m_transactions, m_finishedTransactions;
    };

}

void ReaderStatusThread_sleep( unsigned int secs ) {
    ReaderStatusThread::sleep( secs );
}

class ReaderStatus::Private : ReaderStatusThread {
    friend class Kleo::SmartCard::ReaderStatus;
    ReaderStatus * const q;
public:
    explicit Private( ReaderStatus * qq )
        : ReaderStatusThread( qq ),
          q( qq ),
          watcher()
    {
        KDAB_SET_OBJECT_NAME( watcher );

        qRegisterMetaType<Status>( "Kleo::SmartCard::ReaderStatus::Status" );

        watcher.whitelistFiles( QStringList( QLatin1String( "reader_*.status" ) ) );
        watcher.addPath( Kleo::gnupgHomeDirectory() );
        watcher.setDelay( 100 );

        connect( this, SIGNAL(cardStatusChanged(unsigned int,Kleo::SmartCard::ReaderStatus::Status)),
                 q, SIGNAL(cardStatusChanged(unsigned int,Kleo::SmartCard::ReaderStatus::Status)) );
        connect( this, SIGNAL(anyCardHasNullPinChanged(bool)),
                 q, SIGNAL(anyCardHasNullPinChanged(bool)) );
        connect( this, SIGNAL(anyCardCanLearnKeysChanged(bool)),
                 q, SIGNAL(anyCardCanLearnKeysChanged(bool)) );

        connect( &watcher, SIGNAL(triggered()), this, SLOT(ping()) );

    }
    ~Private() {
        stop();
        if ( !wait( 100 ) ) {
            terminate();
            wait();
        }
    }

private:
    bool anyCardHasNullPinImpl() const {
        return kdtools::any( cardInfos(), bind( &CardInfo::status, _1 ) == CardHasNullPin );
    }

    bool anyCardCanLearnKeysImpl() const {
        return kdtools::any( cardInfos(), bind( &CardInfo::status, _1 ) == CardCanLearnKeys );
    }

private:
    FileSystemWatcher watcher;
};


ReaderStatus::ReaderStatus( QObject * parent )
    : QObject( parent ), d( new Private( this ) )
{
    self = this;
}

ReaderStatus::~ReaderStatus() { self = 0; }

// slot
void ReaderStatus::startMonitoring() {
    d->start();
}

// static
ReaderStatus * ReaderStatus::mutableInstance() {
    return self;
}

// static
const ReaderStatus * ReaderStatus::instance() {
    return self;
}

ReaderStatus::Status ReaderStatus::cardStatus( unsigned int slot ) const {
    return d->cardStatus( slot );
}

bool ReaderStatus::anyCardHasNullPin() const {
    return d->anyCardHasNullPinImpl();
}

bool ReaderStatus::anyCardCanLearnKeys() const {
    return d->anyCardCanLearnKeysImpl();
}

std::vector<ReaderStatus::PinState> ReaderStatus::pinStates( unsigned int slot ) const {
    const std::vector<CardInfo> ci = d->cardInfos();
    if ( slot < ci.size() )
        return ci[slot].pinStates;
    else
        return std::vector<PinState>();
}

void ReaderStatus::startSimpleTransaction( const QByteArray & command, QObject * receiver, const char * slot ) {
    const Transaction t = { command, receiver, slot, Error() };
    d->addTransaction( t );
}

void ReaderStatus::updateStatus() {
    d->ping();
}

#include "moc_readerstatus.cpp"
#include "readerstatus.moc"
