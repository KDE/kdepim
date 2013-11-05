/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifyfilescontroller.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "decryptverifyfilescontroller.h"

#include <crypto/gui/decryptverifyoperationwidget.h>
#include <crypto/gui/decryptverifyfileswizard.h>
#include <crypto/decryptverifytask.h>
#include <crypto/taskcollection.h>

#include <utils/classify.h>
#include <utils/gnupg-helper.h>
#include <utils/path-helper.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>
#include <utils/archivedefinition.h>

#include <KDebug>
#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <memory>
#include <vector>

using namespace boost;
using namespace GpgME;
using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;

class DecryptVerifyFilesController::Private {
    DecryptVerifyFilesController* const q;
public:

    static shared_ptr<AbstractDecryptVerifyTask> taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const QString & fileName, const QDir & outDir, const shared_ptr<OverwritePolicy> & overwritePolicy );

    explicit Private( DecryptVerifyFilesController* qq );

    void slotWizardOperationPrepared();
    void slotWizardCanceled();
    void schedule();

    QStringList prepareWizardFromPassedFiles();
    std::vector<shared_ptr<Task> > buildTasks( const QStringList &, const shared_ptr<OverwritePolicy> & );

    void ensureWizardCreated();
    void ensureWizardVisible();
    void reportError( int err, const QString & details ) {
        q->setLastError( err, details );
        q->emitDoneOrError();
    }
    void cancelAllTasks();

    QStringList m_passedFiles, m_filesAfterPreparation;
    QPointer<DecryptVerifyFilesWizard> m_wizard;
    std::vector<shared_ptr<const DecryptVerifyResult> > m_results;
    std::vector<shared_ptr<Task> > m_runnableTasks, m_completedTasks;
    shared_ptr<Task> m_runningTask;
    bool m_errorDetected;
    DecryptVerifyOperation m_operation;
};

// static
shared_ptr<AbstractDecryptVerifyTask> DecryptVerifyFilesController::Private::taskFromOperationWidget( const DecryptVerifyOperationWidget * w, const QString & fileName, const QDir & outDir, const shared_ptr<OverwritePolicy> & overwritePolicy ) {

    kleo_assert( w );

    shared_ptr<AbstractDecryptVerifyTask> task;

    switch ( w->mode() ) {
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignature:
    {

        shared_ptr<VerifyDetachedTask> t( new VerifyDetachedTask );
        t->setInput( Input::createFromFile( fileName ) );
        t->setSignedData( Input::createFromFile( w->signedDataFileName() ) );
        task = t;

        kleo_assert( fileName == w->inputFileName() );
    }
    break;
    case DecryptVerifyOperationWidget::VerifyDetachedWithSignedData:
    {
        shared_ptr<VerifyDetachedTask> t( new VerifyDetachedTask );
        t->setInput( Input::createFromFile( w->inputFileName() ) );
        t->setSignedData( Input::createFromFile( fileName ) );
        task = t;

        kleo_assert( fileName == w->signedDataFileName() );
    }
    break;
    case DecryptVerifyOperationWidget::DecryptVerifyOpaque:
    {
        const unsigned int classification = classify( fileName );
        kDebug() << "classified" << fileName << "as" << printableClassification( classification );

        const shared_ptr<ArchiveDefinition> ad = w->selectedArchiveDefinition();

        const Protocol proto =
            isOpenPGP( classification ) ? OpenPGP :
            isCMS( classification )     ? CMS :
            ad /* _needs_ the info */   ? throw Exception( gpg_error( GPG_ERR_CONFLICT ), i18n("Cannot determine whether input data is OpenPGP or CMS") ) :
            /* else we don't care */      UnknownProtocol ;

        const shared_ptr<Input> input = Input::createFromFile( fileName );
        const shared_ptr<Output> output =
            ad       ? ad->createOutputFromUnpackCommand( proto, fileName, outDir ) :
            /*else*/   Output::createFromFile( outDir.absoluteFilePath( outputFileName( QFileInfo( fileName ).fileName() ) ), overwritePolicy );

        if ( mayBeCipherText( classification ) ) {
            kDebug() << "creating a DecryptVerifyTask";
            shared_ptr<DecryptVerifyTask> t( new DecryptVerifyTask );
            t->setInput( input );
            t->setOutput( output );
            task = t;
        } else {
            kDebug() << "creating a VerifyOpaqueTask";
            shared_ptr<VerifyOpaqueTask> t( new VerifyOpaqueTask );
            t->setInput( input );
            t->setOutput( output );
            task = t;
        }

        kleo_assert( fileName == w->inputFileName() );
    }
    break;
    }

    task->autodetectProtocolFromInput();
    return task;
}

DecryptVerifyFilesController::Private::Private( DecryptVerifyFilesController* qq ) : q( qq ), m_errorDetected( false ), m_operation( DecryptVerify )
{
    qRegisterMetaType<VerificationResult>();
}

void DecryptVerifyFilesController::Private::slotWizardOperationPrepared()
{
    try {
        ensureWizardCreated();
        std::vector<shared_ptr<Task> > tasks = buildTasks( m_filesAfterPreparation, shared_ptr<OverwritePolicy>( new OverwritePolicy( m_wizard ) ) );
        kleo_assert( m_runnableTasks.empty() );
        m_runnableTasks.swap( tasks );

        shared_ptr<TaskCollection> coll( new TaskCollection );
        Q_FOREACH( const shared_ptr<Task> & i, m_runnableTasks )
            q->connectTask( i );
        coll->setTasks( m_runnableTasks );
        m_wizard->setTaskCollection( coll );

        QTimer::singleShot( 0, q, SLOT(schedule()) );

    } catch ( const Kleo::Exception & e ) {
        reportError( e.error().encodedError(), e.message() );
    } catch ( const std::exception & e ) {
        reportError( gpg_error( GPG_ERR_UNEXPECTED ),
                     i18n("Caught unexpected exception in DecryptVerifyFilesController::Private::slotWizardOperationPrepared: %1",
                          QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        reportError( gpg_error( GPG_ERR_UNEXPECTED ),
                     i18n("Caught unknown exception in DecryptVerifyFilesController::Private::slotWizardOperationPrepared") );
    }
}

void DecryptVerifyFilesController::Private::slotWizardCanceled()
{
    kDebug();
    reportError( gpg_error( GPG_ERR_CANCELED ), i18n("User canceled") );
}

void DecryptVerifyFilesController::doTaskDone( const Task* task, const shared_ptr<const Task::Result> & result )
{
    assert( task );
    assert( task == d->m_runningTask.get() );
    Q_UNUSED( task );

    // We could just delete the tasks here, but we can't use
    // Qt::QueuedConnection here (we need sender()) and other slots
    // might not yet have executed. Therefore, we push completed tasks
    // into a burial container

    d->m_completedTasks.push_back( d->m_runningTask );
    d->m_runningTask.reset();

    if ( const shared_ptr<const DecryptVerifyResult> & dvr = boost::dynamic_pointer_cast<const DecryptVerifyResult>( result ) )
        d->m_results.push_back( dvr );

    QTimer::singleShot( 0, this, SLOT(schedule()) );
}

void DecryptVerifyFilesController::Private::schedule()
{
    if ( !m_runningTask && !m_runnableTasks.empty() ) {
        const shared_ptr<Task> t = m_runnableTasks.back();
        m_runnableTasks.pop_back();
        t->start();
        m_runningTask = t;
    }
    if ( !m_runningTask ) {
        kleo_assert( m_runnableTasks.empty() );
        Q_FOREACH ( const shared_ptr<const DecryptVerifyResult> & i, m_results )
            emit q->verificationResult( i->verificationResult() );
        q->emitDoneOrError();
    }
}

void DecryptVerifyFilesController::Private::ensureWizardCreated()
{
    if ( m_wizard )
        return;

    std::auto_ptr<DecryptVerifyFilesWizard> w( new DecryptVerifyFilesWizard );
    w->setWindowTitle( i18n( "Decrypt/Verify Files" ) );
    w->setAttribute( Qt::WA_DeleteOnClose );

    connect( w.get(), SIGNAL(operationPrepared()), q, SLOT(slotWizardOperationPrepared()), Qt::QueuedConnection );
    connect( w.get(), SIGNAL(canceled()), q, SLOT(slotWizardCanceled()), Qt::QueuedConnection );
    m_wizard = w.release();

}

namespace {
    struct FindExtension : std::unary_function<shared_ptr<ArchiveDefinition>,bool> {
        const QString ext;
        const Protocol proto;
        FindExtension( const QString & ext, Protocol proto ) : ext( ext ), proto( proto ) {}
        bool operator()( const shared_ptr<ArchiveDefinition> & ad ) const {
            kDebug() << "   considering" << ( ad ? ad->label() : QLatin1String( "<null>" ) ) << "for" << ext;
            bool result;
            if ( proto == UnknownProtocol )
                result = ad && ( ad->extensions( OpenPGP ).contains( ext, Qt::CaseInsensitive ) || ad->extensions( CMS ).contains( ext, Qt::CaseInsensitive ) );
            else
                result = ad && ad->extensions( proto ).contains( ext, Qt::CaseInsensitive );
            kDebug() << ( result ? "   -> matches" : "   -> doesn't match" );
            return result;
        }
    };
}

shared_ptr<ArchiveDefinition> pick_archive_definition( GpgME::Protocol proto, const std::vector< shared_ptr<ArchiveDefinition> > & ads, const QString & filename ) {
    const QFileInfo fi( outputFileName( filename ) );
    QString extension = fi.completeSuffix();

    if ( extension == QLatin1String("out") ) // added by outputFileName() -> useless
        return shared_ptr<ArchiveDefinition>();

    if ( extension.endsWith( QLatin1String( ".out" ) ) ) // added by outputFileName() -> remove
        extension.chop(4);

    for ( ;; ) {
        const std::vector<shared_ptr<ArchiveDefinition> >::const_iterator it
            = std::find_if( ads.begin(), ads.end(), FindExtension( extension, proto ) );
        if ( it != ads.end() )
            return *it;
        const int idx = extension.indexOf( QLatin1Char('.') );
        if ( idx < 0 )
            return shared_ptr<ArchiveDefinition>();
        extension = extension.mid( idx + 1 );
    }
}


QStringList DecryptVerifyFilesController::Private::prepareWizardFromPassedFiles()
{
    ensureWizardCreated();

    const std::vector< shared_ptr<ArchiveDefinition> > archiveDefinitions = ArchiveDefinition::getArchiveDefinitions();

    QStringList fileNames;
    unsigned int counter = 0;
    Q_FOREACH( const QString & fname, m_passedFiles ) {

        kleo_assert( !fname.isEmpty() );

        const unsigned int classification = classify( fname );
        const Protocol proto = findProtocol( classification );

        if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) || mayBeDetachedSignature( classification ) ) {

            DecryptVerifyOperationWidget * const op = m_wizard->operationWidget( counter++ );
            kleo_assert( op != 0 );

            op->setArchiveDefinitions( archiveDefinitions );

            const QString signedDataFileName = findSignedData( fname );

            // this breaks opaque signatures whose source files still
            // happen to exist in the same directory. Until we have
            // content-based classification, this is the most unlikely
            // case, so that's the case we break. ### FIXME remove when content-classify is done
            if ( mayBeDetachedSignature( classification ) && !signedDataFileName.isEmpty() )
                op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignature );
            // ### end FIXME
            else if ( mayBeOpaqueSignature( classification ) || mayBeCipherText( classification ) )
                op->setMode( DecryptVerifyOperationWidget::DecryptVerifyOpaque, pick_archive_definition( proto, archiveDefinitions, fname ) );
            else
                op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignature );

            op->setInputFileName( fname );
            op->setSignedDataFileName( signedDataFileName );

            fileNames.push_back( fname );

        } else {

            // probably the signed data file was selected:
            const QStringList signatures = findSignatures( fname );

            if ( signatures.empty() ) {
                // We are assuming this is a detached signature file, but
                // there were no signature files for it. Let's guess it's encrypted after all.
                // ### FIXME once we have a proper heuristic for this, this should move into
                // classify() and/or classifyContent()
                DecryptVerifyOperationWidget * const op = m_wizard->operationWidget( counter++ );
                kleo_assert( op != 0 );
                op->setArchiveDefinitions( archiveDefinitions );
                op->setMode( DecryptVerifyOperationWidget::DecryptVerifyOpaque, pick_archive_definition( proto, archiveDefinitions, fname ) );
                op->setInputFileName( fname );
                fileNames.push_back( fname );
            } else {

                Q_FOREACH( const QString & s, signatures ) {
                    DecryptVerifyOperationWidget * op = m_wizard->operationWidget( counter++ );
                    kleo_assert( op != 0 );

                    op->setArchiveDefinitions( archiveDefinitions );
                    op->setMode( DecryptVerifyOperationWidget::VerifyDetachedWithSignedData );
                    op->setInputFileName( s );
                    op->setSignedDataFileName( fname );

                    fileNames.push_back( fname );
                }

            }
        }
    }

    kleo_assert( counter == static_cast<unsigned>( fileNames.size() ) );

    if ( !counter )
        throw Kleo::Exception( makeGnuPGError( GPG_ERR_ASS_NO_INPUT ), i18n("No usable inputs found") );

    m_wizard->setOutputDirectory( heuristicBaseDirectory( m_passedFiles ) );
    return fileNames;
}

std::vector< shared_ptr<Task> > DecryptVerifyFilesController::Private::buildTasks( const QStringList & fileNames, const shared_ptr<OverwritePolicy> & overwritePolicy )
{
    const bool useOutDir = m_wizard->useOutputDirectory();
    const QFileInfo outDirInfo( m_wizard->outputDirectory() );

    kleo_assert( !useOutDir || outDirInfo.isDir() );

    const QDir outDir( outDirInfo.absoluteFilePath() );
    kleo_assert( !useOutDir || outDir.exists() );

    std::vector<shared_ptr<Task> > tasks;
    for ( unsigned int i = 0, end  = fileNames.size() ; i != end ; ++i )
        try {
            const QDir fileDir = QFileInfo( fileNames[i] ).absoluteDir();
            kleo_assert( fileDir.exists() );
            tasks.push_back( taskFromOperationWidget( m_wizard->operationWidget( i ), fileNames[i], useOutDir ? outDir : fileDir, overwritePolicy ) );
        } catch ( const GpgME::Exception & e ) {
            tasks.push_back( Task::makeErrorTask( e.error().code(), QString::fromLocal8Bit( e.what() ), fileNames[i] ) );
        }

    return tasks;
}

void DecryptVerifyFilesController::setFiles( const QStringList & files )
{
    d->m_passedFiles = files;
}

void DecryptVerifyFilesController::Private::ensureWizardVisible()
{
    ensureWizardCreated();
    q->bringToForeground( m_wizard );
}

DecryptVerifyFilesController::DecryptVerifyFilesController( QObject* parent ) : Controller( parent ), d( new Private( this ) )
{
}

DecryptVerifyFilesController::DecryptVerifyFilesController( const shared_ptr<const ExecutionContext> & ctx, QObject* parent ) : Controller( ctx, parent ), d( new Private( this ) )
{
}


DecryptVerifyFilesController::~DecryptVerifyFilesController() { kDebug(); }

void DecryptVerifyFilesController::start()
{
    d->m_filesAfterPreparation = d->prepareWizardFromPassedFiles();
    d->ensureWizardVisible();
}

void DecryptVerifyFilesController::setOperation( DecryptVerifyOperation op )
{
    d->m_operation = op;
}

DecryptVerifyOperation DecryptVerifyFilesController::operation() const
{
    return d->m_operation;
}

void DecryptVerifyFilesController::Private::cancelAllTasks() {

    // we just kill all runnable tasks - this will not result in
    // signal emissions.
    m_runnableTasks.clear();

    // a cancel() will result in a call to
    if ( m_runningTask )
        m_runningTask->cancel();
}

void DecryptVerifyFilesController::cancel()
{
    kDebug();
    try {
        d->m_errorDetected = true;
        if ( d->m_wizard )
            d->m_wizard->close();
        d->cancelAllTasks();
    } catch ( const std::exception & e ) {
        kDebug() << "Caught exception: " << e.what();
    }
}

#include "moc_decryptverifyfilescontroller.cpp"
