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

#include <QFileSystemWatcher>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include <boost/static_assert.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace Kleo;
using namespace Kleo::SmartCard;
using namespace GpgME;
using namespace boost;

struct CardInfo {
    CardInfo() : fileName(), status( ReaderStatus::NoCard ) {}
    CardInfo( const QString & fn, ReaderStatus::Status s ) : fileName( fn ), status( s ) {}
    QString fileName;
    ReaderStatus::Status status;
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
}

static QDebug operator<<( QDebug s, const std::vector< std::pair<std::string,std::string> > & v ) {
    typedef std::pair<std::string,std::string> pair;
    s << '(';
    Q_FOREACH( const pair & p, v )
        s << "status(" << QString::fromStdString( p.first ) << ") =" << QString::fromStdString( p.second ) << endl;
    return s << ')';
}

static CardInfo get_more_detailed_status( const QString & fileName, unsigned int idx, const shared_ptr<Context> & gpg_agent ) {
    qDebug() << "get_more_detailed_status(" << fileName << ',' << idx << ',' << gpg_agent.get() << ')';
    if ( idx != 0 || !gpg_agent )
        return CardInfo( fileName, ReaderStatus::CardUsable );
    const AssuanResult serialNoResult = gpg_agent->assuanTransact( "SCD SERIALNO" );
    if ( serialNoResult.error().code() )
        return CardInfo( fileName, ReaderStatus::CardUsable );
    const AssuanResult chvStatusResult = gpg_agent->assuanTransact( "SCD GETATTR CHV-STATUS" );
    if ( chvStatusResult.error().code() )
        return CardInfo( fileName, ReaderStatus::CardUsable );
    std::auto_ptr<AssuanTransaction> chvStatusTransaction = gpg_agent->takeLastAssuanTransaction();
    std::auto_ptr<DefaultAssuanTransaction> chvStatus = dynamic_pointer_cast<DefaultAssuanTransaction>( chvStatusTransaction );
    if ( chvStatus.get() )
        qDebug() << "get_more_detailed_status: data=" << QString::fromStdString( chvStatus->data() ) << endl
                 << "  status:" << chvStatus->statusLines();
    const QStringList pinStates = QString::fromStdString( chvStatus->firstStatusLine( "CHV-STATUS" ) ).split( QLatin1Char( ' ' ), QString::SkipEmptyParts );
    if ( pinStates.contains( QLatin1String( "-4" ) ) )
        return CardInfo( fileName, ReaderStatus::CardHasNullPin );
    // PENDING(marc) check for keys to learn
    return CardInfo( fileName, ReaderStatus::CardUsable );
}

static CardInfo get_card_info( const QString & fileName, unsigned int idx, const shared_ptr<Context> & gpg_agent, ReaderStatus::Status oldStatus ) {
    qDebug() << "get_card_info(" << fileName << ',' << idx << ',' << gpg_agent.get() << ',' << prettyFlags[oldStatus] << ')';
    const ReaderStatus::Status st = read_status( fileName );
    if ( st == ReaderStatus::CardUsable && st != oldStatus && gpg_agent )
        return get_more_detailed_status( fileName, idx, gpg_agent );
    else
        return CardInfo( fileName, st );
}

static std::vector<CardInfo> update_cardinfo( const QString & gnupgHomePath, const shared_ptr<Context> & gpgAgent, const std::vector<CardInfo> & oldCardInfos ) {
    const QDir gnupgHome( gnupgHomePath );
    if ( !gnupgHome.exists() ) {
        qDebug() << "update_cardinfo: gnupg home" << gnupgHomePath << "does not exist!";
        return std::vector<CardInfo>();
    }
    const QStringList files = gnupgHome.entryList( QStringList( QLatin1String( "reader_*.status" ) ), QDir::Files, QDir::Name );

    std::vector<CardInfo> result;
    result.reserve( files.size() );
    Q_FOREACH( const QString & file, files ) {
        bool ok = false;
        const unsigned int idx = parseFileName( file, &ok );
        if ( !ok )
            continue;
        result.resize( idx );
        result.push_back( get_card_info( gnupgHome.absoluteFilePath( file ), idx, gpgAgent,
                                         idx < oldCardInfos.size() ? oldCardInfos[idx].status : ReaderStatus::NoCard ) );
    }
    return result;
}

namespace {
    class ReaderStatusThread : public QThread {
        Q_OBJECT
    public:
        explicit ReaderStatusThread( QObject * parent=0 )
            : QThread( parent ),
              m_gnupgHomePath( Kleo::gnupgHomeDirectory() ),
              m_quit( false ),
              m_pendingPing( true ) // force an initial scan
        {

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

    Q_SIGNALS:
        void anyCardNeedsAttentionChanged( bool );
        void anyCardHasNullPinChanged( bool );
        void anyCardCanLearnKeysChanged( bool );
        void cardStatusChanged( unsigned int, ReaderStatus::Status );

    public Q_SLOTS:
        void ping() {
            qDebug() << "ReaderStatusThread[GUI]::ping()";
            const QMutexLocker locker( &m_mutex );
            m_pendingPing = true;
            m_waitForPing.wakeOne();
        }

        void stop() {
            m_quit = true;
            ping();
        }

    private:
        /* reimp */ void run() {
            const shared_ptr<Context> gpgAgent = Context::createForEngine( AssuanEngine );

            while ( true ) {

                bool quit;
                bool pending;
                std::vector<CardInfo> oldCardInfos;

                KDAB_SYNCHRONIZED( m_mutex ) {

                    if ( !m_pendingPing ) {
                        // go to sleep waiting for more work:
                        qDebug( "ReaderStatusThread[2nd]: .zZZ" );
                        m_waitForPing.wait( &m_mutex );
                        qDebug( "ReaderStatusThread[2nd]: .oOO" );
                    }

                    // make local copies so we can release the mutex again:

                    quit = m_quit;
                    pending = m_pendingPing;
                    oldCardInfos = m_cardInfos;

                    // reset the pending flag:
                    m_pendingPing = false;
                }

                qDebug() << "ReaderStatusThread[2nd]: new iteration quit=" << quit << " ; pending=" << pending;

                // now, let's see what we got:

                if ( quit )
                    return;

                if ( !pending )
                    continue;

                // the actual implemementation (the rest is boilerplate code):
                std::vector<CardInfo> newCardInfos
                    = update_cardinfo( m_gnupgHomePath, gpgAgent, oldCardInfos );

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

                emit anyCardCanLearnKeysChanged( anyLC );
                emit anyCardHasNullPinChanged( anyNP );
                emit anyCardNeedsAttentionChanged( anyNP || anyLC );
            }
        }

    private:
        mutable QMutex m_mutex;
        QWaitCondition m_waitForPing;
        const QString m_gnupgHomePath;
        // protected by m_mutex:
        bool m_quit;
        bool m_pendingPing;
        std::vector<CardInfo> m_cardInfos;
    };

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

        watcher.whitelistFiles( QStringList( QLatin1String( "reader_*.status" ) ) );
        watcher.addPath( Kleo::gnupgHomeDirectory() );
        watcher.setDelay( 100 );

        connect( this, SIGNAL(cardStatusChanged(unsigned int,ReaderStatus::Status)),
                 q, SIGNAL(cardStatusChanged(unsigned int,ReaderStatus::Status)) );
        connect( this, SIGNAL(anyCardNeedsAttentionChanged(bool)),
                 q, SIGNAL(anyCardNeedsAttentionChanged(bool)) );
        connect( this, SIGNAL(anyCardHasNullPinChanged(bool)),
                 q, SIGNAL(anyCardHasNullPinChanged(bool)) );
        connect( this, SIGNAL(anyCardCanLearnKeysChanged(bool)),
                 q, SIGNAL(anyCardHasNullPinChanged(bool)) );

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
    bool anyCardNeedsAttentionImpl() const {
        return kdtools::any( cardInfos(), bind( &CardInfo::status, _1 ) > CardUsable );
    }

    bool anyCardHasNullPinImpl() const {
        return kdtools::any( cardInfos(), bind( &CardInfo::status, _1 ) == CardHasNullPin );
    }

private:
    FileSystemWatcher watcher;
};


ReaderStatus::ReaderStatus( QObject * parent )
    : QObject( parent ), d( new Private( this ) )
{
    QTimer::singleShot( 500, d.get(), SLOT(start()) );
}

ReaderStatus::~ReaderStatus() {}

ReaderStatus::Status ReaderStatus::cardStatus( unsigned int slot ) const {
    return d->cardStatus( slot );
}

bool ReaderStatus::anyCardNeedsAttention() const {
    return d->anyCardNeedsAttentionImpl();
}

bool ReaderStatus::anyCardHasNullPin() const {
    return d->anyCardHasNullPinImpl();
}

#include "moc_readerstatus.cpp"
#include "readerstatus.moc"
