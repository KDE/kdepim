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

#include "action_data.h"

#include "searchbar.h"
#include "tabwidget.h"

#include "certificatewizardimpl.h"
#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"
#include "controllers/keylistcontroller.h"
#include "commands/exportcertificatecommand.h"
#include "commands/importcertificatecommand.h"
#include "commands/refreshkeyscommand.h"
#include "commands/detailscommand.h"
#include "commands/deletecertificatescommand.h"

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
using namespace boost;
using namespace GpgME;

namespace {
	QString gpgConfPath()
	{
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
    item.setText( i18n("Only &Close the Window" ) );
    return item;
}

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;

public:
    explicit Private( MainWindow * qq );
    ~Private();

    void closeAndQuit() {
        const QString app = KGlobal::mainComponent().aboutData()->programName();
        const int rc = KMessageBox::questionYesNoCancel( q,
                                                         i18n("%1 may be used by other applications as a service.\n"
                                                              "You may instead want to close this window without exiting %1.", app ),
                                                         i18n("Really quit?"), KStandardGuiItem_quit(), KStandardGuiItem_close(), KStandardGuiItem::cancel(),
                                                         "really-quit-" + app.toLower() );
        if ( rc == KMessageBox::Cancel )
            return;
        q->close();
        // WARNING: 'this' might be deleted at this point!
        if ( rc == KMessageBox::Yes )
            qApp->quit();
    }
    void certificateDetails() {
        ( new DetailsCommand( currentView(), &controller ) )->start();
    }
    void refreshCertificates() {
        ( new RefreshKeysCommand( RefreshKeysCommand::Normal, currentView(), &controller ) )->start();
    }
    void validateCertificates() {
        ( new RefreshKeysCommand( RefreshKeysCommand::Validate, currentView(), &controller ) )->start();
    }
    void deleteCertificates() {
        ( new DeleteCertificatesCommand( currentView(), &controller ) )->start();
    }
    void exportCertificates() {
        ( new ExportCertificateCommand( currentView(), &controller ) )->start();
    }
    void importCertificates() {
        ( new ImportCertificateCommand( currentView(), &controller ) )->start();
    }
    void newCertificate();
    
    void checkConfiguration();
    void configureBackend();

private:
    void setupActions();
    void setupViews();
    void saveViews();

    void addView( const QString & title, const QString & keyFilterID=QString(), const QString & searchString=QString() );
    void addView( const KConfigGroup & group );

    unsigned int numViews() const {
        return ui.tabWidget.count();
    }

    QAbstractItemView * currentView() const {
        return ui.tabWidget.currentView();
    }

private:

    Kleo::AbstractKeyListModel * model;
    Kleo::KeyListController controller;

    QTimer refreshTimer;

    struct Actions {
        //QAction file_new_certificate;
        //QAction file_export_certificate;
        //QAction file_export_secret_keys;
        //QAction file_import_certificates;
        //QAction file_import_crls;
        QAction * file_close;
        QAction * file_quit;
        //QAction view_stop_operations;
        //QAction view_certificate_details;
        //QAction view_hierarchical;
        //QAction view_expandall;
        //QAction view_collapseall;
        QAction * check_configuration;
        QAction * configure_backend;

        explicit Actions( MainWindow * q )
            : file_close( KStandardAction::close( q, SLOT(close()), q->actionCollection() ) ),
              file_quit( KStandardAction::quit( q, SLOT(closeAndQuit()), q->actionCollection() ) ),
              check_configuration( new KAction( q->actionCollection() ) ),
              configure_backend( new KAction( q->actionCollection() ) )
        {
            check_configuration->setText( i18n( "Check GnuPG Configuration..." ) );
            connect( check_configuration, SIGNAL(triggered()), q, SLOT(checkConfiguration()) );
            q->actionCollection()->addAction( "check_configuration", check_configuration );

            configure_backend->setText( i18n("Configure GnuPG Backend..." ) );
            connect( configure_backend, SIGNAL(triggered()), q, SLOT(configureBackend()) );
            q->actionCollection()->addAction( "configure_backend", configure_backend );
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
      model( AbstractKeyListModel::createFlatKeyListModel( q ) ),
      controller( q ),
      refreshTimer(),
      actions( q ),
      ui( q )
{
    KDAB_SET_OBJECT_NAME( controller );
    KDAB_SET_OBJECT_NAME( refreshTimer );

    refreshTimer.setInterval( 5 * 60 * 1000 );
    refreshTimer.setSingleShot( false );
    refreshTimer.start();
    connect( &refreshTimer, SIGNAL(timeout()), q, SLOT(validateCertificates()) );

    controller.setModel( model );
    ui.tabWidget.setModel( model );

    setupActions();

    connect( &controller, SIGNAL(progress(int,int)), &ui.progressBar, SLOT(setProgress(int,int)) );
    connect( &controller, SIGNAL(message(QString,int)),  q->statusBar(), SLOT(showMessage(QString,int)) );

    q->createGUI( "kleopatra_newui.rc" );

    q->resize( 640, 480 );

    setupViews();
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

static void xconnect( const QObject * o1, const char * signal, const QObject * o2, const char * slot ) {
    QObject::connect( o1, signal, o2, slot );
    QObject::connect( o2, signal, o1, slot );
}

void MainWindow::Private::setupActions() {

    KActionCollection * const coll = q->actionCollection();

    QWidgetAction * const searchBarAction = new QWidgetAction( q );
    SearchBar * const searchBar = new SearchBar( q );

    xconnect( searchBar, SIGNAL(stringFilterChanged(QString)),
              &ui.tabWidget, SLOT(setStringFilter(QString)) );
    xconnect( searchBar, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
              &ui.tabWidget, SLOT(setKeyFilter(boost::shared_ptr<Kleo::KeyFilter>)) );
    connect( &ui.tabWidget, SIGNAL(enableChangeStringFilter(bool)),
             searchBar, SLOT(setChangeStringFilterEnabled(bool)) );
    connect( &ui.tabWidget, SIGNAL(enableChangeKeyFilter(bool)),
             searchBar, SLOT(setChangeKeyFilterEnabled(bool)) );

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
        // View menu
        { "view_redisplay", "Redisplay", QString(),
          "view-refresh", q, SLOT(refreshCertificates()), "F5", false, true },
        { "view_stop_operations", i18n( "Stop Operation" ), QString(),
          "process-stop", &controller, SLOT(cancelCommands()), "Escape", false, false },
        { "view_certificate_details", i18n( "Certificate Details..." ), QString(),
          0, q, SLOT(certificateDetails()), QString(), false, true }, // ### should be disabled until selected
        // Certificate menu
        { "certificates_validate", i18n("Validate" ), QString()/*i18n("Validate selected certificates")*/,
          "view-refresh", q, SLOT(validateCertificates()), "SHIFT+F5", false, true },
        { "certificates_delete", i18n("Delete" ), QString()/*i18n("Delete selected certificates")*/,
          "edit-delete", q, SLOT(deleteCertificates()), "Delete", false, true },
        // CRLs menu
        // Tools menu
        // Window menu
        // (come from ui.tabWidget)
    };

    make_actions_from_data( action_data, /*sizeof action_data / sizeof *action_data,*/ coll );

    if ( QAction * action = coll->action( "view_stop_operations" ) )
        connect( &controller, SIGNAL(commandsExecuting(bool)), action, SLOT(setEnabled(bool)) );

    ui.tabWidget.createActions( coll );
}

static QStringList extractViewGroups( const KSharedConfig::Ptr & config ) {
    return config->groupList().filter( QRegExp( "^View #\\d+$" ) );
}

void MainWindow::Private::setupViews() {

    if ( const KSharedConfig::Ptr config = KGlobal::config() ) {

        const QStringList groups = extractViewGroups( config );

        Q_FOREACH( const QString & groupName, groups ) {
            const KConfigGroup group( config, groupName );
            addView( group );
        }

    }

    if ( numViews() == 0 ) {

        // add defaults:

        addView( QString(), "my-certificates" );
        addView( QString(), "trusted-certificates" );
        addView( QString(), "other-certificates" );

    }

}

void MainWindow::Private::saveViews() {

    KSharedConfig::Ptr config = KGlobal::config();
    if ( !config )
        return;

    Q_FOREACH( QString group, extractViewGroups( config ) )
        config->deleteGroup( group );

    for ( unsigned int i = 0, end = numViews() ; i != end ; ++i ) {
        KConfigGroup group( config, QString().sprintf( "View #%u", i ) );
        ui.tabWidget.saveTab( i, group );
    }

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
    QVBoxLayout vlay( &dlg );
    QLabel label( i18n("This is the result of the GnuPG config check:" ), &dlg );
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

    if ( process.error() != QProcess::UnknownError )
        textEdit.setPlainText( QString::fromUtf8( process.readAll() ) + '\n' + process.errorString() );
    else
        textEdit.setPlainText( QString::fromUtf8( process.readAll() ) );

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

void MainWindow::closeEvent( QCloseEvent * e ) {
    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    qDebug( "MainWindow::closeEvent()" );
    d->saveViews();
    e->accept();
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
