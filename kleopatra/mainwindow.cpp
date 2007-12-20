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

#include "mainwindow.h"

#include "searchbar.h"
#include "tabwidget.h"

#include "certificatewizardimpl.h"
#include "searchbarstatehandler.h"
#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"
#include "controllers/keylistcontroller.h"
#include "commands/exportcertificatecommand.h"
#include "commands/importcertificatecommand.h"
#include "commands/refreshkeyscommand.h"
#include "commands/detailscommand.h"

#include <KActionCollection>
#include <KLocale>
#include <KTabWidget>
#include <KStatusBar>
#include <KStandardAction>
#include <KAction>
#include <KAboutData>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <QAbstractItemView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QToolBar>
#include <QWidgetAction>
#include <QProgressBar>
#include <QApplication>
#include <QCloseEvent>
#include <QTimer>

#include <gpgme++/key.h>

#include <boost/shared_ptr.hpp>

#include <vector>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace {
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
    void exportCertificates() {
        ( new ExportCertificateCommand( currentView(), &controller ) )->start();
    }
    void importCertificates() {
        ( new ImportCertificateCommand( currentView(), &controller ) )->start();
    }
    void newCertificate();

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

        explicit Actions( MainWindow * q )
            : file_close( KStandardAction::close( q, SLOT(close()), q->actionCollection() ) ),
              file_quit( KStandardAction::quit( q, SLOT(closeAndQuit()), q->actionCollection() ) )
        {

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
    QAbstractItemView * const view = ui.tabWidget.addView( model, title, id, text );
    assert( view );
    controller.addView( view );
}

void MainWindow::Private::addView( const KConfigGroup & group ) {
    if ( QAbstractItemView * const view = ui.tabWidget.addView( model, group ) )
        controller.addView( view );
}
    
void MainWindow::Private::setupActions() {

    KActionCollection * const coll = q->actionCollection();

    QWidgetAction * const searchBarAction = new QWidgetAction( q );
    SearchBar * const searchBar = new SearchBar( q );
    new SearchBarStateHandler( &ui.tabWidget, searchBar, searchBar );
    connect( searchBar, SIGNAL(textChanged(QString)),
             &ui.tabWidget, SLOT(setStringFilter(QString)) );
    connect( searchBar, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
             &ui.tabWidget, SLOT(setKeyFilter(boost::shared_ptr<Kleo::KeyFilter>)) );
    searchBarAction->setDefaultWidget( searchBar );
    coll->addAction( "key_search_bar", searchBarAction );

    QAction* action;

    // File menu

    action = coll->addAction( "file_new_certificate" );
    action->setText( i18n( "New Certificate..." ) );
    action->setIcon( KIcon( "document-new" ) );
    connect( action, SIGNAL(triggered()), q, SLOT(newCertificate()) );
    action->setShortcuts( KShortcut( i18n("Ctrl+N") ) );

    action = coll->addAction( "file_export_certificates" );
    action->setText( i18n( "Export Certificates..." ) );
    action->setIcon( KIcon("document-export") );
    connect( action, SIGNAL(triggered()), q, SLOT(exportCertificates()) );
    action->setShortcuts( KShortcut( i18n("Ctrl+E") ) );

    action = coll->addAction( "file_import_certificates" );
    action->setText( i18n( "Import Certificates..." ) );
    connect( action, SIGNAL(triggered()), q, SLOT(importCertificates()) );
    action->setShortcuts( KShortcut( i18n("Ctrl+I") ) );

    // View menu

    action = coll->addAction( "view_redisplay" );
    action->setText( i18n( "Redisplay" ) );
    action->setIcon( KIcon( "view-refresh" ) );
    connect( action, SIGNAL(triggered()), q, SLOT(refreshCertificates()) );
    action->setShortcuts( KShortcut( i18n("F5") ) );

    action = coll->addAction( "view_stop_operations" );
    action->setText( i18n( "Stop Operation" ) );
    action->setIcon( KIcon( "process-stop" ) );
    connect( action, SIGNAL(triggered()), &controller, SLOT(cancelCommands()) );
    connect( &controller, SIGNAL(commandsExecuting(bool)), action, SLOT(setEnabled(bool)) );
    action->setShortcuts( KShortcut( i18n("Escape") ) );
    action->setEnabled( false );

    action = coll->addAction( "view_certificate_details" );
    action->setText( i18n( "Certificate Details..." ) );
    //action->setIcon( KIcon( "view-details" ) );
    connect( action, SIGNAL(triggered()), q, SLOT(certificateDetails()) );

    // Certificates menu

    action = coll->addAction( "certificates_validate" );
    action->setText( i18n("Validate" ) );
    action->setIcon( KIcon( "view-refresh" ) );
    //action->setToolTip( i18n("Validate selected certificates") );
    connect( action, SIGNAL(triggered()), q, SLOT(validateCertificates()) );
    action->setShortcuts( KShortcut( i18n("SHIFT+F5") ) );

    // CRLs menu

    // Tools menu

    // Settings menu

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

void MainWindow::closeEvent( QCloseEvent * e ) {
    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    qDebug( "MainWindow::closeEvent()" );
    d->saveViews();
    e->accept();
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
