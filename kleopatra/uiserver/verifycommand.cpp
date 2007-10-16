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
#include "resultdialog.h"
#include "resultdisplaywidget.h"
#include "classify.h"

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>

#include <gpgme++/data.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <KDebug>
#include <KLocale>
#include <kiconloader.h>

#include <QFileDialog>
#include <QObject>
#include <QIODevice>
#include <QVariant>
#include <QDialog>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>

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
    void showResult( int , const VerificationResultCollector::Result & );
    
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


class VerificationResultDisplayWidget : public ResultDisplayWidget
{
public:
    VerificationResultDisplayWidget( QWidget * parent )
        : ResultDisplayWidget( parent )
    {
        m_box = new QVBoxLayout( this );
        m_box->setContentsMargins( 0, 0, 0, 0 );
    }
    void setResult( const GpgME::VerificationResult& result, const std::vector<GpgME::Key> & keys )
    {        
        if ( result.error() ) {
            QString l = "<qt><img src=\"";
            l += KIconLoader::global()->iconPath( "dialog-error", KIconLoader::Small );
            l += "\"/> <b>";
            l += i18n( "Verification failed: " );
            l += result.error().asString();
            l += "</b></qt>";
            QLabel *label = new QLabel(l);
            m_box->addWidget( label );
            setColor( Qt::red );
            return;
        }
        
        while ( QLayoutItem * child = m_box->takeAt(0) )
            delete child;
        
        std::vector<GpgME::Signature> sigs = result.signatures();
        Q_FOREACH ( const GpgME::Signature sig, sigs ) {
            SignatureDisplayWidget * w = new SignatureDisplayWidget( this );
            w->setSignature( sig, keyForSignature( sig, keys ) );
            m_box->addWidget( w );
        }
    }
private:
    QVBoxLayout * m_box;
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
    ResultDialog<VerificationResultDisplayWidget> * dialog;
    VerifyCommand * q;
    VerificationResultCollector * collector;

    QList<AssuanCommandPrivateBase::Input> analyzeInput() const;
    void startVerification();

    void writeOpaqueResult(const GpgME::VerificationResult &, const QByteArray &, int);
    void trySendingStatus( const char * tag, const QString & str );
    QString signatureToString( const GpgME::Signature& sig, const GpgME::Key & key ) const;
    void showVerificationResultDialog();

public Q_SLOTS:
    void verificationFinished( const QHash<int, VerificationResultCollector::Result> & results );
    void slotProgress( const QString& what, int current, int total );
    void slotShowResult( int, const VerificationResultCollector::Result& );
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
           const GpgME::Error verificationError = vResult.error();
           if ( !verificationError ) {
               if ( result.isOpaque )
                   m_command->writeOpaqueResult( vResult, result.stuff, result.id );

               const std::vector<GpgME::Signature> sigs = vResult.signatures();
               assert( !sigs.empty() );
               QStringList resultStrings;
               Q_FOREACH ( const GpgME::Signature & sig, sigs ) {
                   const QString s = m_command->signatureToString( sig, keyForSignature( sig, result.keys ) );
                   resultStrings.append( s );
                   m_command->trySendingStatus( "SIGSTATUS", s );
               }

               resultString = "OK " + resultStrings.join("\n");
           } else {
               resultString = QString::fromLatin1( "ERR %1 - ").arg( QString::number( verificationError ) ) + QString::fromLocal8Bit( verificationError.asString() );
           }
       } catch ( const assuan_exception& e ) {
           result.error = e.error_code();
           result.errorString = e.message();
           m_results[result.id] = result;
           resultString = "ERR " + QString::fromLocal8Bit( e.what() );
           // FIXME ask to continue or cancel
       }
       emit showResult( m_statusSent, result );
       m_command->trySendingStatus( "VERIFY", resultString );
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

QList<AssuanCommandPrivateBase::Input> VerifyCommand::Private::analyzeInput() const
{
    const unsigned int numSignatures = q->numBulkInputDevices( "INPUT" );
    const unsigned int numMessages = q->numBulkInputDevices( "MESSAGE" );
    const unsigned int numOutputs  = q->numBulkOutputDevices( "OUTPUT" );

    if ( !numSignatures )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n("At least one INPUT needs to be provided") );

    if ( numMessages && numMessages != numSignatures )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_INPUT ),  //TODO use better error code if possible
                                i18n("INPUT/MESSAGE count mismatch") );

    if ( numOutputs && numOutputs != numSignatures )
        throw assuan_exception( q->makeError( GPG_ERR_ASS_NO_OUTPUT ), //TODO use better error code if possible
                                i18n("INPUT/OUTPUT count mismatch") );

    if ( numOutputs && numMessages )
        throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                i18n("Can't use OUTPUT and MESSAGE simultaneously") );

    if ( !q->senders().empty() )
        throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                i18n("Can't use SENDER") );

    if ( !q->recipients().empty() )
        throw assuan_exception( q->makeError( GPG_ERR_CONFLICT ),
                                i18n("Can't use RECIPIENT") );

    QList<Input> inputs;

    if ( numMessages )
        // detached signatures:
        for ( unsigned int i = 0; i < numSignatures; ++i ) {

            Input input;
            input.type = Input::Detached;
            input.signature = q->bulkInputDevice( "INPUT", i );
            input.signatureFileName = q->bulkInputDeviceFileName( "INPUT", i );
            input.setupMessage( q->bulkInputDevice( "MESSAGE", i ), q->bulkInputDeviceFileName( "MESSAGE", i ) );
            assuan_assert( input.message || !input.messageFileName.isEmpty() );
            assuan_assert( input.signature );

            inputs.append( input );
        }
        return inputs;

    assuan_assert( numMessages == 0 );

    for ( unsigned int i = 0; i < numSignatures; ++i )
    {
        Input input;
        input.signature = q->bulkInputDevice( "INPUT", i );
        assuan_assert( input.signature );
        const QString fname = q->bulkInputDeviceFileName( "INPUT", i );

        if ( fname.isEmpty() || isOpaqueSignature( fname ) ) {
            // 1. FD and no MESSAGE - can only be opaque
            // 2. heuristics say it's opaque
            input.type = Input::Opaque;
            input.signatureFileName = fname;
            inputs.push_back( input );
        } else if ( isDetachedSignature( fname ) ) {
            // heuristics say it's a detached signature

            QString signedData = findSignedData( fname );
            if ( signedData.isEmpty() )
                // guessing failed, ask the user to supply one
                signedData = QFileDialog::getOpenFileName( 0, i18n("Select Signed File for %1", fname) );
            if ( signedData.isEmpty() )
                continue;

            input.type = Input::Detached;
            input.messageFileName = signedData;
            input.signatureFileName = fname;

            inputs.push_back( input );
        } else {
            // probably the signed data file was selected:
            QStringList signatures = findSignatures( fname );
            const QFileInfo fi( fname );
            if ( signatures.empty() )
                // guesing failed, ask the user to supply the signature
                signatures = QFileDialog::getOpenFileNames( 0, i18n("Please select the signature files corresponding to the data file %1", fi.fileName() ), fi.path() );

            Q_FOREACH( const QString s, signatures ) {
                Input i = input;
                i.type = Input::Detached;
                i.signatureFileName = s;
                i.messageFileName = fname;
                inputs.push_back( i );
            }
        }
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

void VerifyCommand::Private::trySendingStatus( const char * tag, const QString & str )
{
    if ( const int err = q->sendStatus( tag, str ) )
        q->done( err, i18n("Problem writing out verification status.") );
    // ### FIXME: how is the caller supposed to learn about this error?
}

void VerifyCommand::Private::slotDialogClosed()
{
    // FIXME if there was at least one error, and if so declare the whole thing failed.
    q->done();
}

void VerifyCommand::Private::showVerificationResultDialog()
{

    QStringList inputLabels;
    Q_FOREACH( Input i, inputList ) {
        inputLabels.append( i18n("Verifying signature: %1", i.signatureFileName.isEmpty()? "<unnamed input stream>" : i.signatureFileName ) );
    }
    dialog = new ResultDialog<VerificationResultDisplayWidget>( 0, inputLabels ); // fixme opaque parent handle from command line?
    connect( dialog, SIGNAL( accepted() ), this, SLOT( slotDialogClosed() ) );
    connect( dialog, SIGNAL( rejected() ), this, SLOT( slotDialogClosed() ) );
    
    connect( collector, SIGNAL( showResult( int, VerificationResultCollector::Result ) ),
             this, SLOT( slotShowResult( int, VerificationResultCollector::Result ) ) );
    
    dialog->show();
}

void VerifyCommand::Private::slotShowResult( int id, const VerificationResultCollector::Result& res )
{
    if ( res.error ) {
        QString error = i18n("Verification failed. - ") + res.errorString;
        
        if ( inputList.size() > id ) {
            if( !inputList[id].signatureFileName.isEmpty() )
                error += "\nSignature file: " + inputList[id].signatureFileName;
            if( !inputList[id].messageFileName.isEmpty() )
                error += "\nInput file: " + inputList[id].messageFileName;
        }
        qWarning() << "SBOW WARNING: " << error;
        dialog->showError( id, error );
    } else {
        VerificationResultDisplayWidget * w = dialog->widget( id );
        w->setResult( res.result, res.keys );
        dialog->showResultWidget( id );
    }
    
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

void VerifyCommand::Private::startVerification()
{
    assuan_assert( !inputList.isEmpty() );
    connect( collector, SIGNAL( finished( QHash<int, VerificationResultCollector::Result> ) ),
             SLOT( verificationFinished( QHash<int, VerificationResultCollector::Result> ) ) );

    int i = 0;
    Q_FOREACH ( const Private::Input input, inputList ) {

        assuan_assert( input.backend );

        if ( input.type == Private::Input::Opaque ) {

            //fire off appropriate kleo verification job
            VerifyOpaqueJob * const job = input.backend->verifyOpaqueJob();
            assuan_assert(job);
            collector->registerJob( i, job );
            //FIXME: readAll() save enough?
            if ( const GpgME::Error error = job->start( input.signature->readAll() ) )
                throw assuan_exception( error, i18n("Failed to start verification") );

        } else {

            //fire off appropriate kleo verification job
            VerifyDetachedJob * const job = input.backend->verifyDetachedJob();
            assuan_assert(job);
            collector->registerJob( i, job );

            //FIXME: readAll save enough?
            const QByteArray signature = input.signature->readAll();
            assuan_assert( input.message );
            const QByteArray message = input.message->readAll();
            if ( const GpgME::Error error = job->start( signature, message ) )
                throw assuan_exception( error, i18n("Failed to start verification") );

        }
        ++i;
    }

}

int VerifyCommand::doStart()
{
    d->inputList = d->analyzeInput();
    if ( d->inputList.empty() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n("No usable inputs found") );

    d->determineInputsAndProtocols();

    try {
        d->startVerification();
        if( !hasOption("silent") )
            d->showVerificationResultDialog();
        return 0;
    } catch ( ... ) {
        d->showVerificationResultDialog();
        throw;
    }
}

void VerifyCommand::doCanceled()
{
    delete d->dialog;
    d->dialog = 0;
}


#include "verifycommand.moc"
