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

#include <kleo/cryptobackendfactory.h>
#include <kleo/keylistjob.h>

#include <qgpgme/eventloopinteractor.h>

#include <KLocale>

#include <QDebug>
#include <QLineEdit>
#include <QMenuBar>
#include <QStringList>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;
private:
    explicit Private( MainWindow * qq ) : q( qq ) {}
    ~Private() {}
    void setupMenu();
    void startKeyListing( const char* backend ); 
private:
    Kleo::AbstractKeyListModel * model;
    Kleo::KeyListController controller;
    ::Relay* relay;
};

void MainWindow::Private::setupMenu()
{
    QMenu* viewMenu = q->menuBar()->addMenu( i18n("View") );
    viewMenu->addAction( i18n("Refresh Key List"), q, SLOT( listKeys() ) );
}

void MainWindow::Private::startKeyListing( const char* backend )
{
    Kleo::KeyListJob *keylisting = Kleo::CryptoBackendFactory::instance()->protocol( backend )->keyListJob();
    QObject::connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
             relay, SLOT( keyListingDone( GpgME::KeyListResult ) ) );
    QObject::connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
             relay, SLOT( nextKey( GpgME::Key ) ) );
    keylisting->start( QStringList() ); 

}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags ), d( new MainWindow::Private( this ) ) {
    setWindowTitle( i18n("Kleopatra") );

    QWidget* flatWidget = new QWidget( parent );
    QVBoxLayout* flatLayout = new QVBoxLayout( flatWidget );
    QLineEdit* flatLE = new QLineEdit;
    QTreeView* flatListView = new QTreeView;
    flatListView->setSortingEnabled( true );
    flatListView->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    flatLayout->addWidget( flatLE );
    flatLayout->addWidget( flatListView );
    
    Kleo::KeyListSortFilterProxyModel* flatProxy = new Kleo::KeyListSortFilterProxyModel;
    connect( flatLE, SIGNAL( textChanged( QString ) ), flatProxy, SLOT( setFilterFixedString( QString ) ) );

    setCentralWidget( flatWidget );

    d->relay = new ::Relay( this );

    if ( Kleo::AbstractKeyListModel * const model = Kleo::AbstractKeyListModel::createFlatKeyListModel( flatListView ) ) {
        connect( d->relay, SIGNAL( nextKeys( std::vector<GpgME::Key> ) ), model, SLOT( addKeys( std::vector<GpgME::Key> ) ) );
        flatProxy->setSourceModel( model );
        flatListView->setModel( flatProxy );
    }
/*
    QWidget* hierarchicalWidget = new QWidget( parent );
    QVBoxLayout* hierarchicalLayout = new QVBoxLayout( hierarchicalWidget );
    QLineEdit* hierarchicalLE = new QLineEdit;
    QTreeView* hierarchicalListView = new QTreeView;
    hierarchicalListView->setSortingEnabled( true );
    hierarchicalListView->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );

    hierarchicalLayout->addWidget( hierarchicalLE );
    hierarchicalLayout->addWidget( hierarchicalListView );
*/
    d->setupMenu();
    QTimer* keyListTimer = new QTimer( this );
    connect( keyListTimer, SIGNAL( timeout() ), SLOT( listKeys() ) ); 
    keyListTimer->setInterval( 3 * 60 * 1000 );
    keyListTimer->start();
    QTimer::singleShot( 0, this, SLOT( listKeys() ) );
}

MainWindow::~MainWindow() {
    delete d;
}

void MainWindow::listKeys() {
    d->startKeyListing( "openpgp" );
    d->startKeyListing( "smime" );
}

TrayIconListener::TrayIconListener( QWidget* mainWindow, QObject* parent ) : QObject( parent ), m_mainWindow( mainWindow )
{
    assert( m_mainWindow );
}

void TrayIconListener::activated( QSystemTrayIcon::ActivationReason reason )
{
    if ( reason == QSystemTrayIcon::Trigger ) {
        m_mainWindow->setVisible( !m_mainWindow->isVisible() );
    }
}

#include "mainwindow_p.moc"
#include "mainwindow.moc"
