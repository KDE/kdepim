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

#include <KActionCollection>
#include <KLocale>
#include <KTabWidget>
#include <KStatusBar>
#include <KStandardAction>
#include <KAction>

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

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;

public:
    explicit Private( MainWindow * qq );
    ~Private();

    void refreshCertificates();
    void exportCertificates();
    void importCertificates();
    void newCertificate();

private:
    void setupActions();
    void addView( const QString& title );

private:

    Kleo::AbstractKeyListModel * model;
    Kleo::KeyListController controller;

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
              file_quit( KStandardAction::quit( qApp, SLOT(quit()), q->actionCollection() ) )
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
      actions( q ),
      ui( q )
{
    KDAB_SET_OBJECT_NAME( controller );

    controller.setModel( model );
    setupActions();

    connect( &controller, SIGNAL(progress(int,int)), &ui.progressBar, SLOT(setProgress(int,int)) );
    connect( &controller, SIGNAL(message(QString,int)),  q->statusBar(), SLOT(showMessage(QString,int)) );

    q->createGUI( "kleopatra_newui.rc" );

    addView( i18n( "My Certificates" ) );
    addView( i18n( "Trusted Certificates" ) );
    addView( i18n( "All Certificates" ) );

    ( new RefreshKeysCommand( &controller ) )->start();

    q->resize( 640, 480 );
} 

MainWindow::Private::~Private() {} 

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags )
    : KXmlGuiWindow( parent, flags ), d( new Private( this ) )
{

}

MainWindow::~MainWindow() {}


void MainWindow::Private::addView( const QString& title )
{
    QAbstractItemView * const view = ui.tabWidget.addView( model, title );
    assert( view );
    controller.addView( view );
}
    
void MainWindow::Private::setupActions()
{
    KActionCollection * const coll = q->actionCollection();

    QWidgetAction * const searchBarAction = new QWidgetAction( q );
    SearchBar * const searchBar = new SearchBar( q );
    new SearchBarStateHandler( &ui.tabWidget, searchBar, searchBar );
    connect( searchBar, SIGNAL( textChanged( QString ) ),
             &ui.tabWidget, SLOT( setFilter( QString ) ) );
    searchBarAction->setDefaultWidget( searchBar );
    coll->addAction( "key_search_bar", searchBarAction );

    QAction* action = coll->addAction( "file_import_certificates" );
    action->setText( i18n( "Import Certificates..." ) );
    connect( action, SIGNAL( triggered( bool ) ), q, SLOT( importCertificates() ) );
    action->setShortcuts( KShortcut( "Ctrl+I" ) );

    action = coll->addAction( "file_new_certificate" );
    action->setText( i18n( "New Certificate..." ) );
    action->setIcon( KIcon( "document-new" ) );
    connect( action, SIGNAL( triggered( bool ) ), q, SLOT( newCertificate() ) );
    action->setShortcuts( KShortcut( "Ctrl+N" ) );

    action = coll->addAction( "file_export_certificates" );
    action->setText( i18n( "Export Certificates..." ) );
    connect( action, SIGNAL( triggered( bool ) ), q, SLOT( exportCertificates() ) );
    action->setShortcuts( KShortcut( "Ctrl+E" ) );

    action = coll->addAction( "view_refresh_certificate_list" );
    action->setText( i18n( "Refresh Certificate List..." ) );
    connect( action, SIGNAL( triggered( bool ) ), q, SLOT( refreshCertificates() ) );
    action->setShortcuts( KShortcut( "F5" ) );
   
}

void MainWindow::Private::refreshCertificates()
{
    ( new RefreshKeysCommand( &controller ) )->start();
}

void MainWindow::Private::newCertificate()
{
    QPointer<CertificateWizardImpl> wiz( new CertificateWizardImpl( q ) );
    wiz->exec();
    delete wiz;
}

void MainWindow::Private::exportCertificates()
{
     ExportCertificateCommand* const cmd = new ExportCertificateCommand( &controller );
     cmd->setParentWidget( q );
     cmd->setCertificates( controller.model()->keys( ui.tabWidget.currentView()->selectionModel()->selectedRows() ) );
     cmd->start();
}

void MainWindow::Private::importCertificates()
{
    ImportCertificateCommand* const cmd = new ImportCertificateCommand( &controller );
    cmd->setParentWidget( q );
    cmd->start();
}

void MainWindow::closeEvent( QCloseEvent * e ) {
    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    e->accept();
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
