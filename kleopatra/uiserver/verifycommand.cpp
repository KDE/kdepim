/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/verifycommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "verifycommand.h"
#include "assuancommandprivatebase_p.h"

#include <QObject>
#include <QIODevice>
#include <QVariant>
#include <QDialog>
#include <QVBoxLayout>

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>

#include <KDebug>
#include <KLocale>

#include <gpgme++/data.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "signaturedisplaywidget.h"

using namespace Kleo;

// -- helpers ---
static
bool keyMatchesFingerprint( const GpgME::Key & key, const char* fingerprint )
{
      return  qstricmp ( key.keyID(), fingerprint ) == 0 ||
              qstricmp ( key.shortKeyID(), fingerprint ) == 0 ||
              qstricmp ( key.primaryFingerprint(), fingerprint ) == 0;

}

static
GpgME::Key findKeyForSignature( const GpgME::Signature& sig, const std::vector<GpgME::Key>& keys )
{
    std::vector<GpgME::Key>::const_iterator it =
        std::find_if( keys.begin(), keys.end(),
                      boost::bind( keyMatchesFingerprint, _1, sig.fingerprint() ) );
    return it == keys.end() ? GpgME::Key() : *it;
}


class VerificationResultDialog : public QDialog
{
    Q_OBJECT
public:
    VerificationResultDialog( const GpgME::VerificationResult& result, const std::vector<GpgME::Key>& keys )
    {
        QVBoxLayout *box = new QVBoxLayout( this );
        Q_FOREACH( GpgME::Signature sig, result.signatures() ) {
            SignatureDisplayWidget *w = new SignatureDisplayWidget( this );
            w->setSignature( sig, findKeyForSignature( sig, keys ) );
            box->addWidget( w );
        }
    }
    virtual ~VerificationResultDialog() {}
};

class VerificationResultCollector : public QObject
{
    Q_OBJECT
public:
    explicit VerificationResultCollector( QObject* parent = 0 );

    struct Result {
        bool isOpaque;
        QString id;
        GpgME::VerificationResult result;
        QByteArray stuff;
        std::vector<GpgME::Key> keys;
    };

    void registerJob( const QString& id, VerifyDetachedJob* job );
    void registerJob( const QString& id, VerifyOpaqueJob* job );

Q_SIGNALS:
    void finished( const QHash<QString, VerificationResultCollector::Result> & );

private Q_SLOTS:
    void slotVerifyOpaqueResult(const GpgME::VerificationResult &, const QByteArray &, const std::vector<GpgME::Key> & );
    void slotVerifyDetachedResult(const GpgME::VerificationResult &, const std::vector<GpgME::Key> & );

private:
    void addResult( const VerificationResultCollector::Result &res );
    QHash<QString, Result> m_results;
    QHash<QObject*, QString> m_senderToId;
    int m_unfinished;
};

VerificationResultCollector::VerificationResultCollector( QObject* parent ) : QObject( parent ), m_unfinished( 0 )
{
}

void VerificationResultCollector::registerJob( const QString& id, VerifyDetachedJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult, std::vector<GpgME::Key>)),
             this, SLOT(slotVerifyDetachedResult(GpgME::VerificationResult, std::vector<GpgME::Key>)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void VerificationResultCollector::registerJob( const QString& id, VerifyOpaqueJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray,std::vector<GpgME::Key>)),
             this, SLOT(slotVerifyOpaqueResult(GpgME::VerificationResult,QByteArray,std::vector<GpgME::Key>)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void VerificationResultCollector::addResult( const VerificationResultCollector::Result &res )
{
    m_results[res.id] = res;

    --m_unfinished;
    assert( m_unfinished >= 0 );
    if ( m_unfinished == 0 )
    {
        emit finished( m_results );
        deleteLater();
    }
}



void VerificationResultCollector::slotVerifyOpaqueResult(const GpgME::VerificationResult & result,
                                                         const QByteArray & stuff,
                                                         const std::vector<GpgME::Key> &keys)
{
    const QString id = m_senderToId[sender()];

    Result res;
    res.id = id;
    res.isOpaque = true;
    res.stuff = stuff;
    res.result = result;
    addResult( res );
}

void VerificationResultCollector::slotVerifyDetachedResult(const GpgME::VerificationResult & result,
                                                           const std::vector<GpgME::Key> & keys )
{
    const QString id = m_senderToId[sender()];

    Result res;
    res.id = id;
    res.isOpaque = false;
    res.keys = keys;
    res.result = result;
    addResult( res );
}


class VerifyCommand::Private
  : public AssuanCommandPrivateBaseMixin<VerifyCommand::Private, VerifyCommand>
{
    Q_OBJECT
public:
    Private( VerifyCommand * qq )
    :AssuanCommandPrivateBaseMixin<VerifyCommand::Private, VerifyCommand>(), showDetails(false), dialog(0), q( qq )
    {}
    ~Private()
    {
        delete dialog;
    }

    bool showDetails;
    VerificationResultDialog * dialog;
    std::vector<GpgME::Key> keys;
    GpgME::VerificationResult result;
    VerifyCommand * q;

    QList<AssuanCommandPrivateBase::Input> analyzeInput( GpgME::Error& error, QString& errorDetails ) const;
    int startVerification();

    void verifyOpaqueResult(const GpgME::VerificationResult &, const std::vector<GpgME::Key> &, const QByteArray &);
    void verifyDetachedResult(const GpgME::VerificationResult &, const std::vector<GpgME::Key> & );
public Q_SLOTS:
    void verificationFinished( const QHash<QString, VerificationResultCollector::Result> & results ); 
    void slotProgress( const QString& what, int current, int total );
    void parseCommandLine( const std::string & line );
private Q_SLOTS:
    void slotDialogClosed();
private:
    int sendBriefResult() const;
    QString processSignature( const GpgME::Signature& sig, const GpgME::Key & key ) const;
    void showVerificationResultDialog();
    GpgME::Key keyForSignature( const GpgME::Signature& sig ) const;
};

VerifyCommand::VerifyCommand()
    : AssuanCommandMixin<VerifyCommand>(),
      d( new Private( this ) )
{
}

VerifyCommand::~VerifyCommand() {}

QList<AssuanCommandPrivateBase::Input> VerifyCommand::Private::analyzeInput( GpgME::Error& error, QString& errorDetails ) const
{
    error = GpgME::Error();
    errorDetails = QString();

    const int numSignatures = q->numBulkInputDevices( "INPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    if ( numSignatures == 0 )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT );
        errorDetails = "At least one signature must be provided";
        return QList<Input>();
    }

    if ( numMessages > 0 && numMessages != numSignatures )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "The number of MESSAGE inputs must be either equal to the number of signatures or zero";
        return QList<Input>();
    }

    QList<Input> inputs;

    if ( numMessages == numSignatures )
    {
        for ( int i = 0; i < numSignatures; ++i )
        {
            Input input;
            input.type = Input::Detached;
            input.signature = q->bulkInputDevice( "INPUT", i );
            input.signatureFileName = q->bulkInputDeviceFileName( "INPUT", i );
            input.setupMessage( q->bulkInputDevice( "MESSAGE", i ), q->bulkInputDeviceFileName( "MESSAGE", i ) );
            assert( input.message || !input.messageFileName.isEmpty() );
            assert( input.signature );
            inputs.append( input );
        }
        return inputs;
    }

    assert( numMessages == 0 );

    for ( int i = 0; i < numSignatures; ++i )
    {
        Input input;
        input.signature = q->bulkInputDevice( "INPUT", i );
        input.signatureFileName = q->bulkInputDeviceFileName( "INPUT", i );
        assert( input.signature );
        const QString fname = q->bulkInputDeviceFileName( "INPUT", i );
        if ( !fname.isEmpty() && fname.endsWith( ".sig", Qt::CaseInsensitive )
                || fname.endsWith( ".asc", Qt::CaseInsensitive ) )
        { //detached signature file
            const QString msgFileName = fname.left( fname.length() - 4 );
            // TODO: handle error if msg file does not exist
            input.type = Input::Detached;
            input.messageFileName = msgFileName;
        }
        else // opaque
        {
            input.type = Input::Opaque;
        }
        inputs.append( input );
    }
    return inputs;
}

void VerifyCommand::Private::Input::setupMessage( QIODevice* _message, const QString& fileName )
{
#if 0 // MARC_REFACTOR_ME
    message = fileName.isEmpty() ? _message : 0;
#else
    message = _message;
#endif
    messageFileName = fileName;
}

static QString summaryToString( const GpgME::Signature::Summary summary )
{
    QString result;
    if ( summary & GpgME::Signature::Green )
        result = "GREEN ";
    else if ( summary & GpgME::Signature::Red )
        result = "RED ";
    else
        result = "YELLOW ";
    return result;
}

static QString keyToString( const GpgME::Key key )
{
   QString result;
   if ( !key.isNull() ) {
      if ( key.numUserIDs() ) {
         GpgME::UserID id = key.userID(0);
         result = QString::fromUtf8( id.name() ) + " <" + QString::fromUtf8( id.email() ) + ">";
      }
   }
   return result;
}


GpgME::Key VerifyCommand::Private::keyForSignature( const GpgME::Signature& sig ) const
{
    return findKeyForSignature( sig, keys );
}

QString VerifyCommand::Private::processSignature( const GpgME::Signature& sig, const GpgME::Key & key ) const
{
    // FIXME review, should we continue, here?
    if ( sig.isNull() ) {
        q->done( makeError(GPG_ERR_GENERAL) );
        return QString();
    }
    QString sigString = summaryToString( sig.summary() );
    if ( sig.summary() & GpgME::Signature::Red ) {
        sigString += i18n(" The signature by %1 is invalid. Reason given: %2.", keyToString( key ), sig.status().asString() );
    } else if ( !( sig.summary() & GpgME::Signature::Green )  && !( sig.summary() & GpgME::Signature::Red )) {
       if ( key.isNull() )
          sigString += i18n(" The signature could not be verified. Reason given: %1.", sig.status().asString() );
       else
          sigString += i18n(" The signature by %1 could not be verified. Reason given: %2.", keyToString( key ), sig.status().asString() );
    } else {
        sigString += i18n(" The signature by %1 is valid.", keyToString( key ) );
    }
    return sigString;
}

int VerifyCommand::Private::sendBriefResult() const
{
    if ( result.isNull() )
        return makeError(GPG_ERR_GENERAL);

    if ( result.error() )
        return result.error();

    std::vector<GpgME::Signature> sigs = result.signatures();
    if ( sigs.size() == 0 )
        return makeError(GPG_ERR_GENERAL);

    QStringList resultStrings;
    Q_FOREACH ( const GpgME::Signature sig, sigs )
        resultStrings.append( processSignature( sig, keyForSignature( sig ) ) );

    if ( const int err = q->sendStatus( "VERIFY", resultStrings.join("\n") ) )
        return err;
    return 0;
}

void VerifyCommand::Private::slotDialogClosed()
{
    assert(dialog);
    delete dialog;
    dialog = 0;
    q->done();
}

void VerifyCommand::Private::showVerificationResultDialog()
{
    dialog = new VerificationResultDialog( result, keys );
    connect( dialog, SIGNAL( accepted() ), this, SLOT( slotDialogClosed() ) );
    connect( dialog, SIGNAL( rejected() ), this, SLOT( slotDialogClosed() ) );
    dialog->show();
}

void VerifyCommand::Private::verifyOpaqueResult( const GpgME::VerificationResult & _result ,
                                                 const std::vector<GpgME::Key> & _keys,
                                                 const QByteArray & stuff )
{
    result = _result;
    keys = _keys;

    if ( const int err = sendBriefResult() ) {
        q->done( err );
        return;
    }
    QIODevice * const outdevice = q->bulkOutputDevice( "OUTPUT" );
    if ( outdevice ) {
        if ( const int err = outdevice->write( stuff ) ) {
            q->done( err );
            return;
        }
    }

    if ( showDetails )
        showVerificationResultDialog();

    q->done();
}

void VerifyCommand::Private::verificationFinished( const QHash<QString, VerificationResultCollector::Result> & results )
{
    //TODO: handle all results, not only the first
    assert( !results.isEmpty() );
    const VerificationResultCollector::Result result = results.values().first();
    if ( result.isOpaque )
        verifyOpaqueResult( result.result, result.keys, result.stuff );
    else
        verifyDetachedResult( result.result, result.keys );
}

void VerifyCommand::Private::verifyDetachedResult( const GpgME::VerificationResult & _result,
                                                       const std::vector<GpgME::Key>& _keys )
{
    keys = _keys;
    result = _result;

    if ( const int err = sendBriefResult() ) {
        q->done( err );
        return;
    }

    if ( showDetails )
        showVerificationResultDialog();
    else
        q->done();
}

void VerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void VerifyCommand::Private::parseCommandLine( const std::string & line )
{
}

int VerifyCommand::Private::startVerification()
{
    assert( !inputList.isEmpty() );
    VerificationResultCollector* collector = new VerificationResultCollector;
    connect( collector, SIGNAL( finished( QHash<QString, VerificationResultCollector::Result> ) ),
             SLOT( verificationFinished( QHash<QString, VerificationResultCollector::Result> ) ) );

    try {

        int i = 0;
        Q_FOREACH ( const Private::Input input, inputList )
        {
            ++i;
            assert( input.backend );
            if ( input.type == Private::Input::Opaque )
            {
                //fire off appropriate kleo verification job
                VerifyOpaqueJob * const job = input.backend->verifyOpaqueJob();
                assert(job);
                collector->registerJob( QString::number( i ), job );
                //FIXME: readAll() save enough?
                const GpgME::Error error = job->start( input.signature->readAll() );
                if ( error ) throw error;
            }
            else
            {
                //fire off appropriate kleo verification job
                VerifyDetachedJob * const job = input.backend->verifyDetachedJob();
                assert(job);
                collector->registerJob( QString::number( i ), job );

                //FIXME: readAll save enough?
                const QByteArray signature = input.signature->readAll();
                assert( input.message || !input.messageFileName.isEmpty() );
                //const bool useFileName = !input.messageFileName.isEmpty();
                const bool useFileName = false; // FIXME MARC_REFACTOR_ME
                GpgME::Data fileData;
                fileData.setFileName( input.messageFileName.toLatin1().data() );         //FIXME: handle file name encoding correctly 
                assert( !useFileName || !fileData.isNull() );
                const GpgME::Error error = useFileName ? job->start( signature, fileData ) : job->start( signature, input.message->readAll() );
                if ( error ) throw error;
            }
        }
    } catch ( const GpgME::Error & error ) {
        delete collector;
        q->done( error );
        return error;
    }

    return 0;
}

int VerifyCommand::doStart()
{
    d->parseCommandLine("");
    d->showDetails = !hasOption("silent");

    GpgME::Error error;
    QString details;
    d->inputList = d->analyzeInput( error, details );
    if ( error ) {
        done( error, details );
        return error;
    }

    int err = d->determineInputsAndProtocols( details );
    if ( err )
        done( err, details );
    err = d->startVerification();
    return 0;
}

void VerifyCommand::doCanceled()
{
    if ( d->dialog ) {
        delete d->dialog;
        d->dialog = 0;
    }
}


#include "verifycommand.moc"
