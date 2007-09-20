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
#include "kleo-assuan.h"
#include "signaturedisplaywidget.h"

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>

#include <gpgme++/data.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <KDebug>
#include <KLocale>
#include <KFileDialog>
#include <KUrl>


#include <QObject>
#include <QIODevice>
#include <QVariant>
#include <QDialog>
#include <QVBoxLayout>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <cassert>

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
GpgME::Key keyForSignature( const GpgME::Signature& sig, const std::vector<GpgME::Key>& keys )
{
    std::vector<GpgME::Key>::const_iterator it =
        std::find_if( keys.begin(), keys.end(),
                      boost::bind( keyMatchesFingerprint, _1, sig.fingerprint() ) );
    return it == keys.end() ? GpgME::Key() : *it;
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

class VerificationResultDialog : public QDialog
{
    Q_OBJECT
public:
    VerificationResultDialog( QWidget* parent )
    :QDialog( parent )
    {/*
        QVBoxLayout *box = new QVBoxLayout( this );
        Q_FOREACH( GpgME::Signature sig, result.signatures() ) {
            SignatureDisplayWidget *w = new SignatureDisplayWidget( this );
            w->setSignature( sig, keyForSignature( sig, keys ) );
            box->addWidget( w );
        }
        */
    }
    virtual ~VerificationResultDialog() {}
};

class VerifyCommand::Private;

class VerificationResultCollector : public QObject
{
    Q_OBJECT
public:
    explicit VerificationResultCollector( VerifyCommand::Private* parent = 0 );

    struct Result {
        Result() : id(-1), error(0) {}
        bool isOpaque;
        int id;
        GpgME::VerificationResult result;
        QByteArray stuff;
        std::vector<GpgME::Key> keys;
        int error;
        QString errorString;
    };

    void registerJob( int id, VerifyDetachedJob* job );
    void registerJob( int id, VerifyOpaqueJob* job );

Q_SIGNALS:
    void finished( const QHash<int, VerificationResultCollector::Result> & );

private Q_SLOTS:
    void slotVerifyOpaqueResult(const GpgME::VerificationResult &, const QByteArray &, const std::vector<GpgME::Key> & );
    void slotVerifyDetachedResult(const GpgME::VerificationResult &, const std::vector<GpgME::Key> & );

private:
    void addResult( const VerificationResultCollector::Result &res );
    VerifyCommand::Private *m_command;
    QHash<int, Result> m_results;
    QHash<QObject*, int> m_senderToId;
    int m_unfinished;
    int m_statusSent;
};

class VerifyCommand::Private
  : public AssuanCommandPrivateBaseMixin<VerifyCommand::Private, VerifyCommand>
{
    Q_OBJECT
public:
    Private( VerifyCommand * qq )
    :AssuanCommandPrivateBaseMixin<VerifyCommand::Private, VerifyCommand>()
    , dialog(0), q( qq )
    ,collector( new VerificationResultCollector(this) )
    {}
    virtual ~Private() {}
    VerificationResultDialog * dialog;
    VerifyCommand * q;
    VerificationResultCollector * collector;

    QList<AssuanCommandPrivateBase::Input> analyzeInput( GpgME::Error& error, QString& errorDetails ) const;
    int startVerification();

    void writeOpaqueResult(const GpgME::VerificationResult &, const QByteArray &, int);
    void trySendingStatus( const QString & str );
    QString signatureToString( const GpgME::Signature& sig, const GpgME::Key & key ) const;
    void showVerificationResultDialog();

public Q_SLOTS:
    void verificationFinished( const QHash<int, VerificationResultCollector::Result> & results ); 
    void slotProgress( const QString& what, int current, int total );
private Q_SLOTS:
    void slotDialogClosed();
};

VerificationResultCollector::VerificationResultCollector( VerifyCommand::Private* parent ) 
: QObject( parent ), m_command( parent ), m_unfinished( 0 ), m_statusSent( 0 )
{
}

void VerificationResultCollector::registerJob( int id, VerifyDetachedJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult, std::vector<GpgME::Key>)),
             this, SLOT(slotVerifyDetachedResult(GpgME::VerificationResult, std::vector<GpgME::Key>)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void VerificationResultCollector::registerJob( int id, VerifyOpaqueJob* job )
{
    connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray,std::vector<GpgME::Key>)),
             this, SLOT(slotVerifyOpaqueResult(GpgME::VerificationResult,QByteArray,std::vector<GpgME::Key>)) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void VerificationResultCollector::addResult( const VerificationResultCollector::Result &res )
{
    m_results[res.id] = res;

    // send status for all results received so far, but in order of id
    while ( m_results.contains( m_statusSent ) ) {
       Result result = m_results[m_statusSent];
       const GpgME::VerificationResult & vResult = result.result;
       QString resultString;
       try {
           if ( result.isOpaque )
               m_command->writeOpaqueResult( vResult, result.stuff, result.id );

           const GpgME::Error verificationError = vResult.error();
           if ( verificationError )
               throw assuan_exception( verificationError, "Verification failed: " );

           std::vector<GpgME::Signature> sigs = vResult.signatures();
           assert( !sigs.empty() );
           QStringList resultStrings;
           Q_FOREACH ( const GpgME::Signature sig, sigs )
               resultStrings.append( m_command->signatureToString( sig, keyForSignature( sig, result.keys ) ) );

           resultString = "OK " + resultStrings.join("\n");
       } catch ( const assuan_exception& e ) {
           result.error = e.error_code();
           result.errorString = e.what();
           m_results[result.id] = result;
           resultString = "ERR " + result.errorString;
           // FIXME ask to continue or cancel
       }
       m_command->trySendingStatus( resultString );
       m_statusSent++;
    }

    --m_unfinished;
    assert( m_unfinished >= 0 );
    if ( m_unfinished == 0 )
    {
        emit finished( m_results );
    }
}

void VerificationResultCollector::slotVerifyOpaqueResult(const GpgME::VerificationResult & result,
                                                         const QByteArray & stuff,
                                                         const std::vector<GpgME::Key> &keys)
{
    assert( m_senderToId.contains( sender() ) );
    int id = m_senderToId[sender()];

    Result res;
    res.id = id;
    res.isOpaque = true;
    res.stuff = stuff;
    res.keys = keys;
    res.result = result;

    addResult( res );
}

void VerificationResultCollector::slotVerifyDetachedResult(const GpgME::VerificationResult & result,
                                                           const std::vector<GpgME::Key> & keys )
{
    assert( m_senderToId.contains( sender() ) );
    int id = m_senderToId[sender()];

    Result res;
    res.id = id;
    res.isOpaque = false;
    res.keys = keys;
    res.result = result;
    
    addResult( res );
}


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
                || fname.endsWith( ".asc", Qt::CaseInsensitive ) ) {
            //detached signature file
            QString msgFileName = fname.left( fname.length() - 4 );
            QFile f( msgFileName );
            if ( !f.exists() ) {
                // the file we guessed doesn't exist, ask the user to supply one
                const QString userFileName = 
                    KFileDialog::getOpenFileName( KUrl::fromPath(msgFileName), 
                            QString(), 0, i18n("Please select the file corresponding to the signature file: %1", fname) );
                if ( userFileName.isEmpty())
                    continue;
                else
                    msgFileName = userFileName;
            }
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
    message = fileName.isEmpty() ? _message : 0;
    messageFileName = fileName;
}



QString VerifyCommand::Private::signatureToString( const GpgME::Signature& sig, const GpgME::Key & key ) const
{
    if ( sig.isNull() )
        return QString();
 
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

void VerifyCommand::Private::trySendingStatus( const QString & str )
{
    if ( const int err = q->sendStatus( "VERIFY", str ) ) {
        QString errorString = i18n("Problem writing out verification status.");
        q->done( err, errorString ) ;
    }
}

void VerifyCommand::Private::slotDialogClosed()
{
    // FIXME if there was at least one error, and if so declare the whole thing failed.
    q->done();
}

void VerifyCommand::Private::showVerificationResultDialog()
{

    dialog = new VerificationResultDialog( 0 ); // fixme opaque parent handle from command line?
    connect( dialog, SIGNAL( accepted() ), this, SLOT( slotDialogClosed() ) );
    connect( dialog, SIGNAL( rejected() ), this, SLOT( slotDialogClosed() ) );
    dialog->show();
}

void VerifyCommand::Private::writeOpaqueResult( const GpgME::VerificationResult & result ,
                                                const QByteArray & stuff,
                                                int id )
{    
    writeToOutputDeviceOrAskForFileName( id, stuff, result.fileName() );
}

void VerifyCommand::Private::verificationFinished( const QHash<int, VerificationResultCollector::Result> & results )
{
    assert( !results.isEmpty() );

    if ( q->hasOption("silent") ) //otherwise we'll be ending when the dialog closes
        q->done();
}



void VerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

int VerifyCommand::Private::startVerification()
{
    assert( !inputList.isEmpty() );
    connect( collector, SIGNAL( finished( QHash<int, VerificationResultCollector::Result> ) ),
             SLOT( verificationFinished( QHash<int, VerificationResultCollector::Result> ) ) );

    try {

        int i = 0;
        Q_FOREACH ( const Private::Input input, inputList )
        {
            assert( input.backend );
            if ( input.type == Private::Input::Opaque )
            {
                //fire off appropriate kleo verification job
                VerifyOpaqueJob * const job = input.backend->verifyOpaqueJob();
                assert(job);
                collector->registerJob( i, job );
                //FIXME: readAll() save enough?
                const GpgME::Error error = job->start( input.signature->readAll() );
                if ( error ) throw error;
            }
            else
            {
                //fire off appropriate kleo verification job
                VerifyDetachedJob * const job = input.backend->verifyDetachedJob();
                assert(job);
                collector->registerJob( i, job );

                //FIXME: readAll save enough?
                const QByteArray signature = input.signature->readAll();
                assert( input.message || !input.messageFileName.isEmpty() );
                const bool useFileName = !input.messageFileName.isEmpty();
                GpgME::Data fileData;
                fileData.setFileName( input.messageFileName.toLatin1().data() );
                //FIXME: handle file name encoding correctly 
                assert( !useFileName || !fileData.isNull() );
                const GpgME::Error error = useFileName ? job->start( signature, fileData ) : job->start( signature, input.message->readAll() );
                if ( error ) throw error;
            }
            ++i;
        }
    } catch ( const GpgME::Error & error ) {
        q->done( error );
        return error;
    }

    return 0;
}

int VerifyCommand::doStart()
{
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
    if( !err && !hasOption("silent") )
        d->showVerificationResultDialog();
    return err;
}

void VerifyCommand::doCanceled()
{
    if ( d->dialog ) {
        delete d->dialog;
        d->dialog = 0;
    }
}


#include "verifycommand.moc"
