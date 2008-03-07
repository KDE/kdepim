/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow.cpp

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

#include <config-kleopatra.h>

#include "mainwindow.h"

#include "certificatewizardimpl.h"

#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"

#include "view/searchbar.h"
#include "view/tabwidget.h"
#include "view/keylistcontroller.h"

#include "commands/exportcertificatecommand.h"
#include "commands/importcertificatecommand.h"
#include "commands/refreshkeyscommand.h"
#include "commands/detailscommand.h"
#include "commands/deletecertificatescommand.h"
#include "commands/signencryptfilescommand.h"
#include "commands/clearcrlcachecommand.h"
#include "commands/dumpcrlcachecommand.h"
#include "commands/importcrlcommand.h"

#include "conf/configuredialog.h"

#include "utils/stl_util.h"
#include "utils/action_data.h"

#include <KActionCollection>
#include <KLocale>
#include <KTabWidget>
#include <KStatusBar>
#include <KStandardAction>
#include <KAction>
#include <KAboutData>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KStandardDirs>
#include <KShortcutsDialog>
#include <kdebug.h>

#include <QAbstractItemView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QToolBar>
#include <QWidgetAction>
#include <QProgressBar>
#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QProcess>
#include <QTimer>
#include <QMenu>

#include <kleo/cryptobackendfactory.h>
#include <ui/cryptoconfigdialog.h>
#include <kleo/cryptoconfig.h>

#include <gpgme++/engineinfo.h>
#include <gpgme++/global.h>
#include <gpgme++/key.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace boost;
using namespace GpgME;

namespace {
    QString gpgConfPath() {
        const GpgME::EngineInfo info = GpgME::engineInfo( GpgME::GpgConfEngine );
        return info.fileName() ? QFile::decodeName( info.fileName() ) : KStandardDirs::findExe( "gpgconf" );
    }

    class ProgressBar : public QProgressBar {
        Q_OBJECT
    public:
        explicit ProgressBar( QWidget * p=0 ) : QProgressBar( p ) {}

    public Q_SLOTS:
        void setProgress( int current, int total ) {
            setRange( 0, total );
            setValue( current );
        }
    };
}

KGuiItem KStandardGuiItem_quit() {
    static const QString app = KGlobal::mainComponent().aboutData()->programName();
    KGuiItem item = KStandardGuiItem::quit();
    item.setText( i18n( "&Quit %1", app ) );
    return item;
}

KGuiItem KStandardGuiItem_close() {
    KGuiItem item = KStandardGuiItem::close();
    item.setText( i18n("Only &Close Window" ) );
    return item;
}

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;

public:
    explicit Private( MainWindow * qq );
    ~Private();

    template <typename T>
    void createAndStart() {
        ( new T( this->currentView(), &this->controller ) )->start();
    }
    template <typename T, typename A>
    void createAndStart( const A & a ) {
        ( new T( a, this->currentView(), &this->controller ) )->start();
    }

    void closeAndQuit() {
        const QString app = KGlobal::mainComponent().aboutData()->programName();
        const int rc = KMessageBox::questionYesNoCancel( q,
                                                         i18n("%1 may be used by other applications as a service.\n"
                                                              "You may instead want to close this window without exiting %1.", app ),
                                                         i18n("Really Quit?"), KStandardGuiItem_quit(), KStandardGuiItem_close(), KStandardGuiItem::cancel(),
                                                         "really-quit-" + app.toLower() );
        if ( rc == KMessageBox::Cancel )
            return;
        q->close();
        // WARNING: 'this' might be deleted at this point!
        if ( rc == KMessageBox::Yes )
            qApp->quit();
    }
    void certificateDetails() {
        createAndStart<DetailsCommand>();
    }
    void refreshCertificates() {
        createAndStart<RefreshKeysCommand>();
    }
    void deleteCertificates() {
        createAndStart<DeleteCertificatesCommand>();
    }
    void signEncryptFiles() {
        createAndStart<SignEncryptFilesCommand>();
    }
    void exportCertificates() {
        createAndStart<ExportCertificateCommand>();
    }
    void importCertificates() {
        createAndStart<ImportCertificateCommand>();
    }
    void clearCrlCache() {
        createAndStart<ClearCrlCacheCommand>();
    }
    void dumpCrlCache() {
        createAndStart<DumpCrlCacheCommand>();
    }
    void importCrlFromFile() {
        createAndStart<ImportCrlCommand>();
    }
    void editKeybindings() {
        KShortcutsDialog::configure( q->actionCollection(), KShortcutsEditor::LetterShortcutsAllowed );
    }
    void newCertificate();

    void checkConfiguration();
    void configureBackend();
    void preferences();

    void slotConfigCommitted();

private:
    void setupActions();

    void addView( const QString & title, const QString & keyFilterID=QString(), const QString & searchString=QString() );
    void addView( const KConfigGroup & group );

    unsigned int numViews() const {
        return ui.tabWidget.count();
    }

    QAbstractItemView * currentView() const {
        return ui.tabWidget.currentView();
    }

private:

    Kleo::AbstractKeyListModel * flatModel;
    Kleo::AbstractKeyListModel * hierarchicalModel;
    Kleo::KeyListController controller;

    QTimer refreshTimer;
    QPointer<ConfigureDialog> configureDialog;

    struct Actions {
        QAction * check_configuration;
        QAction * configure_backend;

        explicit Actions( MainWindow * q )
            : check_configuration( new KAction( q->actionCollection() ) ),
              configure_backend( new KAction( q->actionCollection() ) )
        {
            check_configuration->setText( i18n( "GnuPG Configuration Self-Check" ) );
            connect( check_configuration, SIGNAL(triggered()), q, SLOT(checkConfiguration()) );
            q->actionCollection()->addAction( "check_configuration", check_configuration );

            configure_backend->setText( i18n("Configure GnuPG Backend..." ) );
            connect( configure_backend, SIGNAL(triggered()), q, SLOT(configureBackend()) );
            q->actionCollection()->addAction( "configure_backend", configure_backend );

            KStandardAction::close( q, SLOT(close()), q->actionCollection() );
            KStandardAction::quit( q, SLOT(closeAndQuit()), q->actionCollection() );
            KStandardAction::keyBindings( q, SLOT(editKeybindings()), q->actionCollection() );
            KStandardAction::preferences( q, SLOT(preferences()), q->actionCollection() );
        }

    } actions;

    struct UI {

        TabWidget tabWidget;
	ProgressBar progressBar;

	explicit UI( MainWindow * q )
	    : tabWidget( q ),
	      progressBar( q->statusBar() )
	{
	    KDAB_SET_OBJECT_NAME( tabWidget );
	    KDAB_SET_OBJECT_NAME( progressBar );

            progressBar.setFixedSize( progressBar.sizeHint() );

            q->setCentralWidget( &tabWidget );
            q->statusBar()->addPermanentWidget( &progressBar );
	}
    } ui;
};

MainWindow::Private::Private( MainWindow * qq )
    : q( qq ),
      flatModel( AbstractKeyListModel::createFlatKeyListModel( q ) ),
      hierarchicalModel( AbstractKeyListModel::createHierarchicalKeyListModel( q ) ),
      controller( q ),
      refreshTimer(),
      configureDialog(),
      actions( q ),
      ui( q )
{
    KDAB_SET_OBJECT_NAME( controller );
    KDAB_SET_OBJECT_NAME( refreshTimer );
    KDAB_SET_OBJECT_NAME( flatModel );
    KDAB_SET_OBJECT_NAME( hierarchicalModel );

    refreshTimer.setInterval( 5 * 60 * 1000 );
    refreshTimer.setSingleShot( false );
    refreshTimer.start();
    connect( &refreshTimer, SIGNAL(timeout()), q, SLOT(refreshCertificates()) );

    controller.setFlatModel( flatModel );
    controller.setHierarchicalModel( hierarchicalModel );

    ui.tabWidget.setFlatModel( flatModel );
    ui.tabWidget.setHierarchicalModel( hierarchicalModel );

    setupActions();

    connect( &controller, SIGNAL(progress(int,int)), &ui.progressBar, SLOT(setProgress(int,int)) );
    connect( &controller, SIGNAL(message(QString,int)),  q->statusBar(), SLOT(showMessage(QString,int)) );

    q->createGUI( "kleopatra_newui.rc" );

    q->setAcceptDrops( true );

    q->setAutoSaveSettings();
}

MainWindow::Private::~Private() {}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags )
    : KXmlGuiWindow( parent, flags ), d( new Private( this ) )
{
}

MainWindow::~MainWindow() {}


void MainWindow::Private::addView( const QString & title, const QString & id, const QString & text ) {
    QAbstractItemView * const view = ui.tabWidget.addView( title, id, text );
    assert( view );
    controller.addView( view );
}

void MainWindow::Private::addView( const KConfigGroup & group ) {
    if ( QAbstractItemView * const view = ui.tabWidget.addView( group ) )
        controller.addView( view );
}

void MainWindow::Private::setupActions() {

    KActionCollection * const coll = q->actionCollection();

    QWidgetAction * const searchBarAction = new QWidgetAction( q );
    SearchBar * const searchBar = new SearchBar( q );

    ui.tabWidget.connectSearchBar( searchBar );

    searchBarAction->setDefaultWidget( searchBar );
    coll->addAction( "key_search_bar", searchBarAction );

    const action_data action_data[] = {
        // File menu
        { "file_new_certificate", i18n("New Certificate..."), QString(),
          "document-new", q, SLOT(newCertificate()), "Ctrl+N", false, true },
        { "file_export_certificates", i18n("Export Certificates..."), QString(),
          "document-export", q, SLOT(exportCertificates()), "Ctrl+E", false, true }, // ### should be disabled until selected
        { "file_import_certificates", i18n("Import Certificates..."), QString(),
          "document-import", q, SLOT(importCertificates()), "Ctrl+I", false, true }, // ### should be disabled until selected
        { "file_sign_encrypt_files", i18n("Sign/Encrypt Files..."), QString(),
          "document-encrypt" /*"file-encrypt-sign"*/, q, SLOT(signEncryptFiles()), QString(), false, true },
        // View menu
        { "view_redisplay", "Redisplay", QString(),
          "view-refresh", q, SLOT(refreshCertificates()), "F5", false, true },
        { "view_stop_operations", i18n( "Stop Operation" ), QString(),
          "process-stop", &controller, SLOT(cancelCommands()), "Escape", false, false },
        { "view_certificate_details", i18n( "Certificate Details" ), QString(),
          0, q, SLOT(certificateDetails()), QString(), false, true }, // ### should be disabled until selected
        // Certificate menu
        { "certificates_delete", i18n("Delete" ), QString()/*i18n("Delete selected certificates")*/,
          "edit-delete", q, SLOT(deleteCertificates()), "Delete", false, true },
        // CRLs menu
        { "crl_clear_crl_cache", i18n("Clear CRL Cache"), QString(),
          0, q, SLOT(clearCrlCache()), QString(), false, true },
        { "crl_dump_crl_cache", i18n("Dump CRL Cache"), QString(),
          0, q, SLOT(dumpCrlCache()), QString(), false, true },
        { "crl_import_crl", i18n("Import CRL From File..."), QString(),
          0, q, SLOT(importCrlFromFile()), QString(), false, true },
        // Tools menu
        // Window menu
        // (come from ui.tabWidget)
    };

    make_actions_from_data( action_data, /*sizeof action_data / sizeof *action_data,*/ coll );

    if ( QAction * action = coll->action( "view_stop_operations" ) )
        connect( &controller, SIGNAL(commandsExecuting(bool)), action, SLOT(setEnabled(bool)) );

    ui.tabWidget.createActions( coll );
}

void MainWindow::Private::newCertificate() {
    QPointer<CertificateWizardImpl> wiz( new CertificateWizardImpl( q ) );
    wiz->exec();
    delete wiz;
}

void MainWindow::Private::checkConfiguration()
{
    assert( actions.check_configuration->isEnabled() );
    if ( !actions.check_configuration->isEnabled() )
        return;

    actions.check_configuration->setEnabled( false );
    const shared_ptr<QAction> enabler( actions.check_configuration, bind( &QAction::setEnabled, _1, true ) );

    // 1. start process
    QProcess process;
    process.setProcessChannelMode( QProcess::MergedChannels );
    process.start( gpgConfPath(), QStringList() << "--check-config", QIODevice::ReadOnly );


    // 2. show dialog:
    QDialog dlg;
    dlg.setWindowTitle( i18n( "GnuPG Configuration Self-Check Results" ) );
    QVBoxLayout vlay( &dlg );
    QLabel label( i18n("This is the result of the GnuPG configuration self-check:" ), &dlg );
    QTextEdit textEdit( &dlg );
    QDialogButtonBox box( QDialogButtonBox::Close, Qt::Horizontal, &dlg );

    textEdit.setReadOnly( true );
    textEdit.setWordWrapMode( QTextOption::NoWrap );

    vlay.addWidget( &label );
    vlay.addWidget( &textEdit, 1 );
    vlay.addWidget( &box );

    dlg.show();

    connect( box.button( QDialogButtonBox::Close ), SIGNAL(clicked()), &dlg, SLOT(reject()) );
    connect( &dlg, SIGNAL(finished(int)), &process, SLOT(terminate()) );

    // 3. wait for either dialog close or process exit
    QEventLoop loop;
    connect( &process, SIGNAL(finished(int,QProcess::ExitStatus)), &loop, SLOT(quit()) );
    connect( &dlg, SIGNAL(finished(int)), &loop, SLOT(quit()) );

    const QPointer<QObject> Q( q );
    loop.exec();

    // safety exit:
    if ( !Q )
        return;

    // check whether it was the dialog that was closed, and return in
    // that case:
    if ( !dlg.isVisible() )
        return;

    const QString output = QString::fromUtf8( process.readAll() );
    const QString message = process.exitStatus() == QProcess::CrashExit ? i18n( "The process terminated prematurely" ) : process.errorString() ;

    if ( process.exitStatus() != QProcess::NormalExit ||
         process.error()      != QProcess::UnknownError )
        textEdit.setPlainText( !output.trimmed().isEmpty()
                               ? i18n( "There was an error executing the GnuPG configuration self-check:\n"
                                       "  %1\n"
                                       "You might want to execute \"gpgconf --check-config\" on the command line.\n"
                                       "\n"
                                       "Diagnostics:", message ) + '\n' + output
                               : i18n( "There was an error executing \"gpgconf --check-config\":\n"
                                       "  %1\n"
                                       "You might want to execute \"gpgconf --check-config\" on the command line.", message ) );
    else if ( process.exitCode() )
        textEdit.setPlainText( !output.trimmed().isEmpty()
                               ? i18n( "The GnuPG configuration self-check failed.\n"
                                       "\n"
                                       "Error code: %1\n"
                                       "Diagnostics:", process.exitCode() ) + '\n' + output
                               : i18n( "The GnuPG configuration self-check failed with error code %1.\n"
                                       "No output was received." ) );
    else
        textEdit.setPlainText( !output.trimmed().isEmpty()
                               ? i18n( "The GnuPG configuration self-check succeeded.\n"
                                       "\n"
                                       "Diagnostics:" ) + '\n' + output
                               : i18n( "The GnuPG configuration self-check succeeded." ) );

    // wait for dialog close:
    assert( dlg.isVisible() );
    loop.exec();
}

void MainWindow::Private::configureBackend() {
    Kleo::CryptoConfig * const config = Kleo::CryptoBackendFactory::instance()->config();
    if ( !config ) {
    	KMessageBox::error( q, i18n( "Could not configure the cryptography backend (gpgconf tool not found)" ), i18n( "Configuration Error" ) );
    	return;
    }

    Kleo::CryptoConfigDialog dlg( config );

    const int result = dlg.exec();

    // Forget all data parsed from gpgconf, so that we show updated information
    // when reopening the configuration dialog.
    config->clear();

    if ( result == QDialog::Accepted ) {
#if 0
        // Tell other apps (e.g. kmail) that the gpgconf data might have changed
        QDBusMessage message =
            QDBusMessage::createSignal(QString(), "org.kde.kleo.CryptoConfig", "changed");
        QDBusConnection::sessionBus().send(message);
#endif
    }
}

void MainWindow::Private::preferences() {
    if ( !configureDialog ) {
        configureDialog = new ConfigureDialog( q );
        configureDialog->setAttribute( Qt::WA_DeleteOnClose );
        connect( configureDialog, SIGNAL(configCommitted()), q, SLOT(slotConfigCommitted()) );
    }

    if ( configureDialog->isVisible() )
        configureDialog->raise();
    else
        configureDialog->show();
}

void MainWindow::Private::slotConfigCommitted() {

}

void MainWindow::closeEvent( QCloseEvent * e ) {
    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    kDebug();
    d->ui.tabWidget.saveViews( KGlobal::config().data() );
    saveMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() ) );
    e->accept();
}

void MainWindow::showEvent( QShowEvent * e ) {
    KXmlGuiWindow::showEvent( e );
    d->ui.tabWidget.loadViews( KGlobal::config().data() );
}

static QStringList extract_local_files( const QMimeData * data ) {
    const QList<QUrl> urls = data->urls();
    // begin workaround KDE/Qt misinterpretation of text/uri-list
    QList<QUrl>::const_iterator end = urls.end();
    if ( urls.size() > 1 && !urls.back().isValid() )
        --end;
    // end workaround
    QStringList result;
    std::transform( urls.begin(), end,
                    std::back_inserter( result ),
                    bind( &QUrl::toLocalFile, _1 ) );
    result.erase( std::remove_if( result.begin(), result.end(),
                                  bind( &QString::isEmpty, _1 ) ), result.end() );
    return result;
}

static bool can_decode_local_files( const QMimeData * data ) {
    if ( !data )
        return false;
    return !extract_local_files( data ).empty();
}

void MainWindow::dragEnterEvent( QDragEnterEvent * e ) {
    kDebug();

    if ( ( e->possibleActions() & Qt::CopyAction ) &&
         can_decode_local_files( e->mimeData() ) )
        e->acceptProposedAction();
}

void MainWindow::dropEvent( QDropEvent * e ) {
    kDebug();

    if ( !( e->possibleActions() & Qt::CopyAction ) ||
         !can_decode_local_files( e->mimeData() ) )
        return;

    e->setDropAction( Qt::CopyAction );

    const QStringList files = extract_local_files( e->mimeData() );

    // ### todo: classify further

    QMenu menu;
    QAction * const signEncrypt = menu.addAction( i18n("Sign/Encrypt...") );
    menu.addSeparator();
    QAction * const importCerts = menu.addAction( i18n("Import Certificates") );
    QAction * const importCRLs  = menu.addAction( i18n("Import CRLs") );
    menu.addSeparator();
    menu.addAction( i18n("Cancel") );

    const QAction * const chosen = menu.exec( mapToGlobal( e->pos() ) );

    if ( chosen == signEncrypt )
        d->createAndStart<SignEncryptFilesCommand>( files );
    else if ( chosen == importCerts )
        d->createAndStart<ImportCertificateCommand>( files );
    else if ( chosen == importCRLs )
        d->createAndStart<ImportCrlCommand>( files );

    if ( chosen )
        e->accept();
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
