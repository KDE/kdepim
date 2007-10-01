/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptemailcommand.cpp

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

#include "decryptcommand.h"
#include "assuancommandprivatebase_p.h"
#include "kleo-assuan.h"
#include "resultdialog.h"
#include "utils/formatting.h"
#include "resultdisplaywidget.h"

#include <QObject>
#include <QIODevice>
#include <QHash>
#include <QMap>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPointer>

#include <kleo/decryptjob.h>
#include <kleo/keylistjob.h>
#include <utils/stl_util.h>

#include <kiconloader.h>
#include <klocale.h>

#include <gpgme++/error.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <boost/bind.hpp>

#include <cassert>

using namespace Kleo;

class DecryptCommand::Private;

class DecryptionResultCollector : public QObject
{
    Q_OBJECT
public:
    explicit DecryptionResultCollector( DecryptCommand::Private* parent = 0 );

    struct Result {
        Result() : error(0) { }
        int id;
        GpgME::DecryptionResult result;
        QByteArray stuff;
        int error;
        QString errorString;
    };

    void registerJob( int id, DecryptJob* job );
    bool hasError() const { return m_hasError; }
Q_SIGNALS:
    void finished( const QMap<int, DecryptionResultCollector::Result> & );
    void showResult( int, const DecryptionResultCollector::Result& );

private Q_SLOTS:
    void slotDecryptResult(const GpgME::DecryptionResult &, const QByteArray &);

private:
    QMap<int, Result> m_results;
    QHash<QObject*, int> m_senderToId;
    int m_unfinished;
    DecryptCommand::Private* m_command;
    int m_statusSent;
    bool m_hasError;
};

class DecryptResultDisplayWidget : public ResultDisplayWidget
{
    Q_OBJECT
public:
    DecryptResultDisplayWidget( QWidget * parent )
    : ResultDisplayWidget( parent ), m_state( Summary ), m_jobCount( 0 )
    {
        QVBoxLayout *layout = new QVBoxLayout( this );
        m_summaryLabel = new QLabel( this );
        layout->addWidget( m_summaryLabel );
        m_detailsLabel = new QLabel( this );
        connect( m_detailsLabel, SIGNAL(linkActivated(QString)), SLOT(linkActivated(QString)) );
        layout->addWidget( m_detailsLabel );
    }

    void setResult( const GpgME::DecryptionResult & result )
    {
        m_result = result;
        reload();
    }

    void setBackends( const QList<const Kleo::CryptoBackend::Protocol*> &backends )
    {
        m_backends = backends;
    }

private:
    void reload()
    {
        if ( m_result.error() ) {
            setColor( Qt::red );
            QString l = "<qt><img src=\"";
            l += KIconLoader::global()->iconPath( "dialog-error", K3Icon::Small );
            l += "\"/> <b>";
            l += i18n( "Decryption failed" );
            l += "</b></qt>";
            m_summaryLabel->setText( l );
        } else {
            setColor( Qt::green );
            QString l = "<qt><img src=\"";
            l += KIconLoader::global()->iconPath( "dialog-ok", K3Icon::Small );
            l += "\"/> <b>";
            l += i18n( "Decryption succeeded" );
            l += "</b></qt>";
            m_summaryLabel->setText( l );
        }
        if ( m_state == Summary )
            m_detailsLabel->setText( "<qt><i><a href=\"showKeys\">" + i18n( "Details..." ) + "</a></i></qt>" );
        else if ( m_state == WaitingForKeys )
            m_detailsLabel->setText( "<qt><i>Retrieving keys...</i></qt>" );
        else if ( m_state == ListingKeysFailed )
            m_detailsLabel->setText( "<qt><i>Retrieving keys failed.</i></qt>" );
        else if ( m_state == Details ) {
            if ( m_result.error() ) {
                m_detailsLabel->setText( QString::fromUtf8("<qt><i> %1.</i></qt>").arg( m_result.error().asString() ) );
            } else {
                if ( m_keys.empty() )
                    m_detailsLabel->setText( i18n("<qt><i>No receipients found.</i></qt>") );
                else {
                    QString l = "<qt>" + i18np( "Receipient:", "Reciepients:", m_result.numRecipients() ) + "<ul>";
                    Q_FOREACH( const GpgME::Key key, m_keys ) {
                        l += "<li>" + renderKey( key ) + "</li>";
                    }
                    if ( m_keys.size() < m_result.numRecipients() ) {
                        l += "<li>" + i18np( "one unknown receipient", "%n unknown receipients",
                                             m_result.numRecipients() - m_keys.size() );
                    }
                    l += "</ul></qt>";
                    m_detailsLabel->setText( l );
                }
            }
        }
    }

private Q_SLOTS:
    void linkActivated( const QString &link )
    {
        if ( link == "showKeys" ) {
            m_state = WaitingForKeys;
            reload();
            QStringList keys;
            Q_FOREACH( const GpgME::DecryptionResult::Recipient r, m_result.recipients() )
                keys << QLatin1String( r.keyID() );
            if ( keys.isEmpty() ) {
                m_state = Details;
                reload();
                return;
            }
            Q_FOREACH( const Kleo::CryptoBackend::Protocol *b, m_backends ) {
                ++m_jobCount;
                Kleo::KeyListJob *job = b->keyListJob();
                connect( job, SIGNAL(nextKey(GpgME::Key)), SLOT(nextKey(GpgME::Key)) );
                connect( job, SIGNAL(result(GpgME::KeyListResult)), SLOT(keyListResult(GpgME::KeyListResult)) );
                job->start( keys, false );
            }
            return;
        } else {
            ResultDisplayWidget::keyLinkActivated( link );
        }
    }

    void nextKey( const GpgME::Key &key )
    {
        m_keys.push_back( key );
    }

    void keyListResult( const GpgME::KeyListResult &result )
    {
        --m_jobCount;
        if ( result.error() ) {
            m_state = ListingKeysFailed;
            reload();
        } else if ( m_jobCount == 0 ) {
            m_state = Details;
            reload();
        }
    }

private:
    QList<const Kleo::CryptoBackend::Protocol*> m_backends;
    GpgME::DecryptionResult m_result;
    std::vector<GpgME::Key> m_keys;
    QLabel *m_summaryLabel;
    QLabel *m_detailsLabel;
    enum State {
        Summary,
        WaitingForKeys,
        ListingKeysFailed,
        Details
    } m_state;
    int m_jobCount;
};

class DecryptCommand::Private
  : public AssuanCommandPrivateBaseMixin<DecryptCommand::Private, DecryptCommand>
{
      friend class DecryptCommand;
    Q_OBJECT
public:
    Private( DecryptCommand * qq )
        :AssuanCommandPrivateBaseMixin<DecryptCommand::Private, DecryptCommand>()
        , q( qq )
        , collector( new DecryptionResultCollector(this) )
    {}

    DecryptCommand *q;
    QList<Input> analyzeInput( GpgME::Error& error, QString& errorDetails ) const;
    int startDecryption();
    bool trySendingStatus( const QString & str );
    void showDecryptResultDialog();

public Q_SLOTS:
    void slotDecryptionCollectionResult( const QMap<int, DecryptionResultCollector::Result>& );
    void slotProgress( const QString& what, int current, int total );
    void slotDialogClosed();
    void slotShowResult( int, const DecryptionResultCollector::Result& );

private:
    DecryptionResultCollector* collector;
    QPointer< ResultDialog<DecryptResultDisplayWidget> > dialog;

};

static QString resultToString( const GpgME::DecryptionResult & result )
{
    if ( result.error() )
        return QString::fromUtf8( "ERR %1 - ").arg( QString::number( result.error() ) ) + result.error().asString();
    QString resStr( "OK ");
    for ( unsigned int i = 0; i<result.numRecipients(); i++ ) {
        const GpgME::DecryptionResult::Recipient r = result.recipient( i );
        resStr += i18n( " encrypted to " ) + QString( r.keyID() );
    }
    return resStr;
}

DecryptionResultCollector::DecryptionResultCollector( DecryptCommand::Private * parent )
: QObject( parent ), m_command( parent ), m_unfinished( 0 ), m_statusSent( 0 ), m_hasError( false )
{
}

void DecryptionResultCollector::registerJob( int id, DecryptJob* job )
{
    connect( job, SIGNAL( result( GpgME::DecryptionResult,QByteArray ) ),
             this, SLOT( slotDecryptResult( GpgME::DecryptionResult, QByteArray ) ) );
    m_senderToId[job] = id;
    ++m_unfinished;
}

void DecryptionResultCollector::slotDecryptResult(const GpgME::DecryptionResult & result,
                                                  const QByteArray & stuff )
{
    static bool mutex = false;
    assert( m_senderToId.contains( sender( ) ) );
    const int id = m_senderToId[sender()];
    if ( result.error() ) 
        m_hasError = true;

    Result res;
    res.id = id;
    res.stuff = stuff;
    res.result = result;
    m_results[id] = res;

    --m_unfinished;
    assert( m_unfinished >= 0 );

    if ( mutex ) return;
    // send status for all results received so far, but in order of id
    // report status on the command line immediately, and write out results, but
     // only show dialog once all operations are completed, so it can be aggregated
    mutex = true;
    while ( m_results.contains( m_statusSent ) ) {
        Result result = m_results[m_statusSent];
        QString resultString;
        try {
            if ( !result.result.error() )
                 m_command->writeToOutputDeviceOrAskForFileName( m_statusSent, result.stuff, result.result.fileName() );
            resultString = resultToString( result.result );
        } catch ( const assuan_exception& e ) {
            result.error = e.error_code();
            result.errorString = QString::fromStdString( e.message() );
            m_results[m_statusSent] = result;
            resultString = "ERR " + QString::fromUtf8( e.what() );
            m_hasError = true;
            // FIXME ask to continue or cancel
        }
        emit showResult( m_statusSent, result );
        if ( !m_command->trySendingStatus( resultString ) ) // will emit done() on error
            return;
        m_statusSent++;
    }

    if ( m_unfinished == 0 ) {
        emit finished( m_results );
    }
    mutex = false;
}

DecryptCommand::DecryptCommand()
    : AssuanCommandMixin<DecryptCommand>(),
      d( new Private( this ) )
{
}

DecryptCommand::~DecryptCommand() {}

QList<AssuanCommandPrivateBase::Input> DecryptCommand::Private::analyzeInput( GpgME::Error& error, QString& errorDetails ) const
{
    error = GpgME::Error();
    errorDetails = QString();

    const int numInputs = q->numBulkInputDevices( "INPUT" );
    const int numOutputs = q->numBulkInputDevices( "OUTPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    if ( numMessages != 0 )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "Only --input can be provided to the decrypt command, no --message";
        return QList<Input>();
    }

    // either the output is discarded, or there ar as many as inputs
    if ( numOutputs > 0 && numInputs != numOutputs )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "For each --input there needs to be an --output";
        return QList<Input>();
    }

    QList<Input> inputs;

    for ( int i = 0; i < numInputs; ++i )
    {
        Input input;
        input.message = q->bulkInputDevice( "INPUT", i );
        input.messageFileName = q->bulkInputDeviceFileName( "INPUT", i );
        assert( input.message );
        input.type = Input::Opaque; // by definition
        inputs.append( input );
    }

    return inputs;
}

bool DecryptCommand::Private::trySendingStatus( const QString & str )
{
    if ( const int err = q->sendStatus( "DECRYPT", str ) ) {
        QString errorString = i18n("Problem writing out decryption status.");
        q->done( err, errorString ) ;
        return false;
    }
    return true;
}

void DecryptCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}


void DecryptCommand::Private::slotDecryptionCollectionResult( const QMap<int, DecryptionResultCollector::Result>& results )
{
    assert( !results.isEmpty() );
    if ( !q->hasOption( "silent" ) ) return; // we're showing a dialog, it will report when done

    if ( collector->hasError() )
        q->done( makeError( GPG_ERR_DECRYPT_FAILED ) );
    else
        q->done();
}

void DecryptCommand::Private::slotDialogClosed()
{
    if ( collector->hasError() )
        q->done( makeError( GPG_ERR_DECRYPT_FAILED ) );
    else
        q->done();
}

void DecryptCommand::Private::slotShowResult( int id, const DecryptionResultCollector::Result &result )
{
    if ( result.error ) {
         dialog->showError( id, result.errorString );
     } else {
         DecryptResultDisplayWidget * w = dialog->widget( id );
         QList<const Kleo::CryptoBackend::Protocol*> backends;
         Q_FOREACH( Input i, inputList ) {
            if ( !backends.contains( i.backend ) )
                backends.push_back( i.backend );
         }
         w->setBackends( backends );
         w->setResult( result.result );
         dialog->showResultWidget( id );
     }
}

void DecryptCommand::Private::showDecryptResultDialog()
{
    QStringList inputLabels;
    Q_FOREACH( Input i, inputList ) {
        inputLabels.append( i18n("Decrypting: %1", i.messageFileName.isEmpty()? "<unnamed input stream>" : i.messageFileName ) );
    }
    dialog = new ResultDialog<DecryptResultDisplayWidget>( 0, inputLabels ); // fixme opaque parent handle from command line?
    connect( dialog, SIGNAL( accepted() ), this, SLOT( slotDialogClosed() ) );
    connect( dialog, SIGNAL( rejected() ), this, SLOT( slotDialogClosed() ) );
    connect( collector, SIGNAL( showResult( int, DecryptionResultCollector::Result ) ),
             this, SLOT( slotShowResult( int, DecryptionResultCollector::Result ) ) );
    dialog->show();
}

int DecryptCommand::Private::startDecryption()
{
    assert( !inputList.isEmpty() );
    connect( collector, SIGNAL( finished( QMap<int, DecryptionResultCollector::Result> ) ),
             SLOT( slotDecryptionCollectionResult( QMap<int, DecryptionResultCollector::Result> ) ) );

    try {
        int i = 0;
        Q_FOREACH ( const Private::Input input, inputList )
        {
            assert( input.backend );

            //fire off appropriate kleo decrypt job
            DecryptJob * const job = input.backend->decryptJob();
            assert(job);
            collector->registerJob( i, job );

            // FIXME handle file names
            const QByteArray encrypted = input.message->readAll(); // FIXME safe enough?
            const GpgME::Error error = job->start( encrypted );
            if ( error ) throw error;

            ++i;
        }
    } catch ( const GpgME::Error & error ) {
        q->done( error );
        return error;
    }

    return 0;
}

int DecryptCommand::doStart()
{
    GpgME::Error error;
    QString details;
    d->inputList = d->analyzeInput( error, details );
    if ( error ) {
        done( error, details );
        return error;
    }

    int err = d->determineInputsAndProtocols( details );
    if ( err ) {
        done( err, details );
        return err;
    }
    err = d->startDecryption();
    if ( !err && !hasOption( "silent" ) )
        d->showDecryptResultDialog();
    return 0;

}

void DecryptCommand::doCanceled()
{
    delete d->dialog;
    d->dialog = 0;
}

#include "decryptcommand.moc"

