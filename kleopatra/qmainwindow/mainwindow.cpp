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
#include "mainwindow_p.h"

#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"
#include "controllers/keylistcontroller.h"
#include "certificateinfowidgetimpl.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/keylistjob.h>

#include <qgpgme/eventloopinteractor.h>

#include <KDialog>
#include <KLocale>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QMenuBar>
#include <QModelIndex>
#include <QStatusBar>
#include <QStringList>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;
private:
    explicit Private( MainWindow * qq ) : q( qq ), model( 0 ), relay( 0 ) {}
    ~Private() {}

    void setupMenu();
    void listKeys();
    void startKeyListing( const char* backend );
    void viewDetails( const GpgME::Key& key );
    void viewDetails( const QModelIndex& idx );

private:
    Kleo::AbstractKeyListModel * model;
    Kleo::KeyListController controller;
    ::Relay* relay;
    ::StatusBarUpdater* statusBarUpdater;
};

void MainWindow::Private::setupMenu()
{
    QMenu* const fileMenu = q->menuBar()->addMenu( i18n("&File") );
    QAction* const quitAction = fileMenu->addAction( i18n("Quit"), qApp, SLOT( quit() ) );
    quitAction->setShortcut( Qt::CTRL + Qt::Key_Q );	
    QMenu* const viewMenu = q->menuBar()->addMenu( i18n("&View") );
    QAction* const refreshAction = viewMenu->addAction( i18n("Refresh Key List"), q, SLOT( listKeys() ) );
    refreshAction->setShortcut( Qt::Key_F5 );
}

void MainWindow::Private::viewDetails( const QModelIndex& idx ) {
    viewDetails( model->key( idx ) );
}

void MainWindow::Private::viewDetails( const GpgME::Key& key ) {
    if ( key.isNull() )
        return;

    KDialog* dialog = new KDialog( q );
    dialog->setObjectName( "dialog" );
    dialog->setModal( false );
    dialog->setWindowTitle( i18n("Additional Information for Certificate") );
    dialog->setButtons( KDialog::Close );
    dialog->setDefaultButton( KDialog::Close );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    CertificateInfoWidgetImpl* top = new CertificateInfoWidgetImpl( key, /*isRemote=*/false, dialog );
    dialog->setMainWidget( top );
#if 0
    q->connect( top, SIGNAL( requestCertificateDownload( QString, QString ) ),
                SLOT( slotStartCertificateDownload( QString, QString ) ) );
#endif
    dialog->show();
}

void MainWindow::Private::startKeyListing( const char* backend )
{
    Kleo::KeyListJob *keylisting = Kleo::CryptoBackendFactory::instance()->protocol( backend )->keyListJob();
    QObject::connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
             relay, SLOT( keyListingDone( GpgME::KeyListResult ) ) );
    QObject::connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
             relay, SLOT( nextKey( GpgME::Key ) ) );
    statusBarUpdater->registerJob( keylisting );
    keylisting->start( QStringList() ); 
}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags ), d( new MainWindow::Private( this ) ) {
    setWindowTitle( i18n("Kleopatra") );

    QWidget* flatWidget = new QWidget( parent );
    QVBoxLayout* flatLayout = new QVBoxLayout( flatWidget );
    QLineEdit* flatLE = new QLineEdit;
    QTreeView* flatListView = new QTreeView;
    flatListView->setRootIsDecorated( false );
    flatListView->setSortingEnabled( true );
    flatListView->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    connect( flatListView, SIGNAL( doubleClicked( QModelIndex ) ), 
             SLOT( viewDetails( QModelIndex ) ) );
    flatLayout->addWidget( flatLE );
    flatLayout->addWidget( flatListView );
    
    Kleo::KeyListSortFilterProxyModel* flatProxy = new Kleo::KeyListSortFilterProxyModel;
    connect( flatLE, SIGNAL( textChanged( QString ) ), flatProxy, SLOT( setFilterFixedString( QString ) ) );

    setCentralWidget( flatWidget );

    d->relay = new ::Relay( this );
    d->statusBarUpdater = new ::StatusBarUpdater( statusBar(), this );

    if ( d->model = Kleo::AbstractKeyListModel::createFlatKeyListModel( flatListView ) ) {
        connect( d->relay, SIGNAL( nextKeys( std::vector<GpgME::Key> ) ), d->model, SLOT( addKeys( std::vector<GpgME::Key> ) ) );
        flatProxy->setSourceModel( d->model );
        flatListView->setModel( flatProxy );
    }

    d->setupMenu();
    QTimer* keyListTimer = new QTimer( this );
    connect( keyListTimer, SIGNAL( timeout() ), SLOT( listKeys() ) ); 
    keyListTimer->setInterval( 30 * 1000 );
    keyListTimer->start();
    QTimer::singleShot( 0, this, SLOT( listKeys() ) );
}

MainWindow::~MainWindow() {
    delete d;
}

void MainWindow::Private::listKeys() {
    startKeyListing( "openpgp" );
    startKeyListing( "smime" );
}

TrayIconListener::TrayIconListener( QWidget* mainWindow, QObject* parent ) : QObject( parent ), m_mainWindow( mainWindow )
{
    assert( m_mainWindow );
}

void TrayIconListener::activated( QSystemTrayIcon::ActivationReason reason )
{
    if ( reason != QSystemTrayIcon::Trigger )
        return;
    const bool visible = !m_mainWindow->isVisible();
    if ( visible ) {
        if ( m_prevGeometry.isValid() )
            m_mainWindow->setGeometry( m_prevGeometry );
        m_mainWindow->setVisible( true );
    } else {
        m_prevGeometry = m_mainWindow->geometry();
        m_mainWindow->setVisible( false );
    }
}

StatusBarUpdater::StatusBarUpdater( QStatusBar* bar, QObject* parent ) : QObject( parent ), m_bar( bar ), m_pendingJobs( 0 ) {
    assert( m_bar );
}

void StatusBarUpdater::registerJob( Kleo::KeyListJob* job ) {
    //TODO: handle errors
    connect( job, SIGNAL( result( GpgME::KeyListResult ) ),
             SLOT( keyListResult( GpgME::KeyListResult ) ) );
    ++m_pendingJobs;
    m_bar->showMessage( i18n("Fetching keys...") );
}
                                             
void StatusBarUpdater::keyListResult( const GpgME::KeyListResult& result ) {
    Q_UNUSED( result );
    --m_pendingJobs;
    assert( m_pendingJobs >= 0 );
    if ( m_pendingJobs == 0 )
        m_bar->clearMessage();
}

#include "mainwindow_p.moc"
#include "mainwindow.moc"
