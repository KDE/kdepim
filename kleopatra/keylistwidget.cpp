/* -*- mode: c++; c-basic-offset:4 -*-
    keylistwidget.cpp

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

#include "keylistwidget.h"
#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"

#include <QGridLayout>
#include <QTreeView>

#include <cassert>

class KeyListWidget::Private {
    friend class ::KeyListWidget;
    KeyListWidget * const q;
public:
    explicit Private( KeyListWidget * qq );
    ~Private();

    QTreeView * m_view;
    Kleo::KeyListSortFilterProxyModel * proxy;
};


KeyListWidget::Private::Private( KeyListWidget * qq )
    : q( qq ), proxy( 0 )
{
    QGridLayout * const layout = new QGridLayout( q );
    layout->setMargin( 0 );
    m_view = new QTreeView;
    m_view->setRootIsDecorated( false );
    m_view->setSortingEnabled( true );
    m_view->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    layout->addWidget( m_view, 0, 0 );
}

KeyListWidget::Private::~Private() {}



KeyListWidget::KeyListWidget( QWidget * parent, Qt::WFlags f )
  : QWidget( parent, f ), d( new Private( this ) )
{
    
}

KeyListWidget::~KeyListWidget() {}

QAbstractItemView* KeyListWidget::view() const
{
    return d->m_view;
}

void KeyListWidget::setModel( QAbstractItemModel* model )
{
    d->proxy = new Kleo::KeyListSortFilterProxyModel( this );
    d->proxy->setSourceModel( model );
    d->m_view->setModel( d->proxy );
}

void KeyListWidget::setKeyFilters( const std::vector< boost::shared_ptr<const KeyFilter> > & kf )
{
    assert( d->proxy );
    d->proxy->setKeyFilters( kf );
}

