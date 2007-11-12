/* -*- mode: c++; c-basic-offset:4 -*-
    controllers/keylistcontroller.cpp

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

#include "keylistcontroller.h"
#include "commands/detailscommand.h"

#include <models/keycache.h>
#include <models/keylistmodel.h>

#include <QAbstractItemView>
#include <QHeaderView>
#include <QTreeView>
#include <QTableView>
#include <QPointer>
#include <QItemSelectionModel>

#include <algorithm>
#include <cassert>

using namespace Kleo;
//using namespace GpgME;

static const QHeaderView::ResizeMode resize_modes[Kleo::AbstractKeyListModel::NumColumns] = {
    QHeaderView::Stretch,          // Name
    QHeaderView::ResizeToContents, // EMail
    QHeaderView::ResizeToContents, // Valid From
    QHeaderView::ResizeToContents, // Valid Until
    QHeaderView::ResizeToContents, // Details
    QHeaderView::ResizeToContents, // Fingerprint
};

static QHeaderView * get_header_view( QAbstractItemView * view ) {
    if ( const QTableView * const table = qobject_cast<QTableView*>( view ) )
        return table->horizontalHeader();
    if ( const QTreeView * const tree = qobject_cast<QTreeView*>( view ) )
        return tree->header();
    return 0;
}

class KeyListController::Private {
    friend class ::Kleo::KeyListController;
    KeyListController * const q;
public:
    explicit Private( KeyListController * qq );
    ~Private();

    void connectView( QAbstractItemView * view );
    void connectCommand( Command * cmd );
    void connectModel();
    void addCommand( Command * cmd ) {
        connectCommand( cmd );
        commands.push_back( cmd );
        std::inplace_merge( commands.begin(), commands.end() - 1, commands.end() );
    }


public:
    void slotDestroyed( QObject * o ) {
        views.erase( std::remove( views.begin(), views.end(), o ), views.end() );
	commands.erase( std::remove( commands.begin(), commands.end(), o ), commands.end() );
    }
    void slotDoubleClicked( const QModelIndex & idx );
    void slotActivated( const QModelIndex & idx );
    void slotSelectionChanged( const QItemSelection & old, const QItemSelection & new_ );
    void slotContextMenu( const QPoint & pos );
    void slotCommandFinished();
    void slotAddKey( const GpgME::Key & key );

private:
    std::vector<QAbstractItemView*> views;
    std::vector<Command*> commands;
    QPointer<AbstractKeyListModel> model;
};


KeyListController::Private::Private( KeyListController * qq )
    : q( qq ),
      views(),
      commands(),
      model( 0 )
{

}

KeyListController::Private::~Private() {}

KeyListController::KeyListController( QObject * p )
    : QObject( p ), d( new Private( this ) )
{
    connect( KeyCache::mutableInstance().get(), SIGNAL( added( GpgME::Key ) ),
             this, SLOT( slotAddKey( GpgME::Key ) ) );
}

KeyListController::~KeyListController() {}



void KeyListController::Private::slotAddKey( const GpgME::Key & key )
{
    if ( !model )
        return;
    model->addKey( key ); 
}

void KeyListController::addView( QAbstractItemView * view ) {
    if ( view && std::binary_search( d->views.begin(), d->views.end(), view ) )
        return;

    // merge 'view' in:
    d->views.push_back( view );
    std::inplace_merge( d->views.begin(), d->views.end() - 1, d->views.end() );

    d->connectView( view );
}

void KeyListController::removeView( QAbstractItemView * view ) {
    if ( !view )
        return;
    view->disconnect( this );
    d->views.erase( std::remove( d->views.begin(), d->views.end(), view ), d->views.end() );
}

std::vector<QAbstractItemView*> KeyListController::views() const {
    return d->views;
}

void KeyListController::setModel( AbstractKeyListModel * model ) {
    if ( model == d->model )
        return;

    if ( d->model )
        d->model->disconnect( this );

    d->model = model;
    
    if ( model )
        d->connectModel();
}

AbstractKeyListModel * KeyListController::model() const {
    return d->model;
}


void KeyListController::Private::connectView( QAbstractItemView * view ) {
    assert( std::binary_search( views.begin(), views.end(), view ) );

    if ( model )
        view->setModel( model );

    connect( view, SIGNAL(destroyed(QObject*)),
             q, SLOT(slotDestroyed(QObject*)) );
    connect( view, SIGNAL(doubleClicked(QModelIndex)),
             q, SLOT(slotDoubleClicked(QModelIndex)) );
    connect( view, SIGNAL(activated(QModelIndex)),
             q, SLOT(slotActivated(QModelIndex)) );
    connect( view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             q, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)) );

    view->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( view, SIGNAL(customContextMenuRequested(QPoint)),
             q, SLOT(slotContextMenu(QPoint)) );

    view->setSelectionBehavior( QAbstractItemView::SelectRows );
    view->setSelectionMode( QAbstractItemView::ExtendedSelection );
    //view->setAlternatingRowColors( true );
    view->setProperty( "allColumnsShowFocus", true );

    if ( QHeaderView * const hv = get_header_view( view ) )
        for ( int i = 0, end = std::min<int>( hv->count(), AbstractKeyListModel::NumColumns ) ; i < end ; ++i )
            hv->setResizeMode( i, resize_modes[i] );
}

void KeyListController::Private::connectCommand( Command * cmd ) {
    if ( !cmd )
        return;
    connect( cmd, SIGNAL(destroyed(QObject*)), q, SLOT(slotDestroyed(QObject*)) );
    connect( cmd, SIGNAL(finished()), q, SLOT(slotCommandFinished()) );
}


void KeyListController::Private::connectModel() {
    if ( !model )
        return;
    // ### anything we want from the model??
}


void KeyListController::Private::slotDoubleClicked( const QModelIndex & idx ) {
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( !view || !std::binary_search( views.begin(), views.end(), view ) )
	return;

    DetailsCommand * const c = new DetailsCommand( q );
    addCommand( c );

    c->setIndex( idx );
    c->setView( view );
    c->start();
}

void KeyListController::Private::slotActivated( const QModelIndex & idx ) {
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( !view || !std::binary_search( views.begin(), views.end(), view ) )
	return;
    
}

void KeyListController::Private::slotSelectionChanged( const QItemSelection & old, const QItemSelection & new_ ) {
    const QItemSelectionModel * const sm = qobject_cast<QItemSelectionModel*>( q->sender() );
    if ( !sm )
	return;
    // ### enable/disable actions
}

void KeyListController::Private::slotContextMenu( const QPoint & p ) {
    QAbstractItemView * const view = qobject_cast<QAbstractItemView*>( q->sender() );
    if ( !view || !std::binary_search( views.begin(), views.end(), view ) )
	return;
    
}

void KeyListController::Private::slotCommandFinished() {
    Command * const cmd = qobject_cast<Command*>( q->sender() );
    if ( !cmd || !std::binary_search( commands.begin(), commands.end(), cmd ) )
        return;
    // ### anything here?
}

#include "moc_keylistcontroller.cpp"
