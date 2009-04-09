/* -*- mode: c++; c-basic-offset:4 -*-
    view/keytreeview.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "keytreeview.h"

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <utils/headerview.h>

#include <kleo/keyfilter.h>

#include <gpgme++/key.h>

#include <QTreeView>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QLayout>

#include <cassert>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace {

class TreeView : public QTreeView {
public:
    explicit TreeView( QWidget * parent=0 ) : QTreeView( parent ) {}

    /* reimp */ QSize minimumSizeHint() const {
        const QSize min = QTreeView::minimumSizeHint();
        return QSize( min.width(), min.height() + 5 * fontMetrics().height() );
    }
};

} // anon namespace

KeyTreeView::KeyTreeView( const KeyTreeView & other )
    : QWidget( 0 ),
      m_proxy( new KeyListSortFilterProxyModel( this ) ),
      m_additionalProxy( other.m_additionalProxy ? other.m_additionalProxy->clone() : 0 ),
      m_view( new TreeView( this ) ),
      m_flatModel( other.m_flatModel ),
      m_hierarchicalModel( other.m_hierarchicalModel ),
      m_stringFilter( other.m_stringFilter ),
      m_keyFilter( other.m_keyFilter ),
      m_isHierarchical( other.m_isHierarchical )
{
    init();
    setColumnSizes( other.columnSizes() );
    setSortColumn( other.sortColumn(), other.sortOrder() );
}

KeyTreeView::KeyTreeView( const QString & text, const shared_ptr<KeyFilter> & kf, AbstractKeyListSortFilterProxyModel * proxy, QWidget * parent )
    : QWidget( parent ),
      m_proxy( new KeyListSortFilterProxyModel( this ) ),
      m_additionalProxy( proxy ),
      m_view( new TreeView( this ) ),
      m_flatModel( 0 ),
      m_hierarchicalModel( 0 ),
      m_stringFilter( text ),
      m_keyFilter( kf ),
      m_isHierarchical( true )
{
    init();
}

void KeyTreeView::setColumnSizes( const std::vector<int> & sizes ) {
    if ( sizes.empty() )
        return;
    assert( m_view );
    assert( m_view->header() );
    assert( qobject_cast<HeaderView*>( m_view->header() ) == static_cast<HeaderView*>( m_view->header() ) );
    if ( HeaderView * const hv = static_cast<HeaderView*>( m_view->header() ) )
        hv->setSectionSizes( sizes );
}

void KeyTreeView::setSortColumn( int sortColumn, Qt::SortOrder sortOrder ) {
    assert( m_view );
    m_view->sortByColumn( sortColumn, sortOrder );
}

int KeyTreeView::sortColumn() const {
    assert( m_view );
    assert( m_view->header() );
    return m_view->header()->sortIndicatorSection();
}

Qt::SortOrder KeyTreeView::sortOrder() const {
    assert( m_view );
    assert( m_view->header() );
    return m_view->header()->sortIndicatorOrder();
}

std::vector<int> KeyTreeView::columnSizes() const {
    assert( m_view );
    assert( m_view->header() );
    assert( qobject_cast<HeaderView*>( m_view->header() ) == static_cast<HeaderView*>( m_view->header() ) );
    if ( HeaderView * const hv = static_cast<HeaderView*>( m_view->header() ) )
        return hv->sectionSizes();
    else
        return std::vector<int>();
}    

void KeyTreeView::init() {
    KDAB_SET_OBJECT_NAME( m_proxy );
    KDAB_SET_OBJECT_NAME( m_view );
    if ( m_additionalProxy && m_additionalProxy->objectName().isEmpty() )
        KDAB_SET_OBJECT_NAME( m_additionalProxy );

    QLayout * layout = new QVBoxLayout( this );
    KDAB_SET_OBJECT_NAME( layout );
    layout->setMargin( 0 );
    layout->addWidget( m_view );

    HeaderView * headerView = new HeaderView( Qt::Horizontal );
    KDAB_SET_OBJECT_NAME( headerView );
    m_view->setHeader( headerView );

    m_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_view->setSelectionMode( QAbstractItemView::ExtendedSelection );
    //m_view->setAlternatingRowColors( true );
    m_view->setAllColumnsShowFocus( true );
    m_view->setSortingEnabled( true );

    if ( model() )
        if ( m_additionalProxy )
            m_additionalProxy->setSourceModel( model() );
        else
            m_proxy->setSourceModel( model() );
    if ( m_additionalProxy ) {
        m_proxy->setSourceModel( m_additionalProxy );
        if ( !m_additionalProxy->parent() )
            m_additionalProxy->setParent( this );
    }
    m_proxy->setFilterFixedString( m_stringFilter );
    m_proxy->setKeyFilter( m_keyFilter );
    m_view->setModel( m_proxy );
}

KeyTreeView::~KeyTreeView() {}

static QAbstractProxyModel * find_last_proxy( QAbstractProxyModel * pm ) {
    assert( pm );
    while ( QAbstractProxyModel * const sm = qobject_cast<QAbstractProxyModel*>( pm->sourceModel() ) )
        pm = sm;
    return pm;
}

void KeyTreeView::setFlatModel( AbstractKeyListModel * model ) {
    if ( model == m_flatModel )
        return;
    m_flatModel = model;
    if ( !m_isHierarchical )
        find_last_proxy( m_proxy )->setSourceModel( model );
}

void KeyTreeView::setHierarchicalModel( AbstractKeyListModel * model ) {
    if ( model == m_hierarchicalModel )
        return;
    m_hierarchicalModel = model;
    if ( m_isHierarchical ) {
        find_last_proxy( m_proxy )->setSourceModel( model );
        m_view->expandAll();
    }
}

void KeyTreeView::setStringFilter( const QString & filter ) {
    if ( filter == m_stringFilter )
        return;
    m_stringFilter = filter;
    m_proxy->setFilterFixedString( filter ); 
    emit stringFilterChanged( filter );
}

void KeyTreeView::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    if ( filter == m_keyFilter || filter && m_keyFilter && filter->id() == m_keyFilter->id() )
        return;
    m_keyFilter = filter;
    m_proxy->setKeyFilter( filter );
    emit keyFilterChanged( filter );
}

static QItemSelection itemSelectionFromKeys( const std::vector<Key> & keys, const KeyListSortFilterProxyModel & proxy ) {
    QItemSelection result;
    Q_FOREACH( const Key & key, keys ) {
        const QModelIndex mi = proxy.index( key );
        if ( mi.isValid() )
            result.merge( QItemSelection( mi, mi ), QItemSelectionModel::Select );
    }
    return result;
}

void KeyTreeView::setHierarchicalView( bool on ) {
    if ( on == m_isHierarchical )
        return;
    const std::vector<Key> selectedKeys = m_proxy->keys( m_view->selectionModel()->selectedRows() );
    const Key currentKey = m_proxy->key( m_view->currentIndex() );

    m_isHierarchical = on;
    find_last_proxy( m_proxy )->setSourceModel( model() );
    if ( on )
        m_view->expandAll();
    m_view->selectionModel()->select( itemSelectionFromKeys( selectedKeys, *m_proxy ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    if ( !currentKey.isNull() ) {
        const QModelIndex currentIndex = m_proxy->index( currentKey );
        if ( currentIndex.isValid() ) {
            m_view->selectionModel()->setCurrentIndex( m_proxy->index( currentKey ), QItemSelectionModel::NoUpdate );
            m_view->scrollTo( currentIndex );
        }
    }
    emit hierarchicalChanged( on );
}

#include "moc_keytreeview.cpp"
