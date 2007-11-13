/* -*- mode: c++; c-basic-offset:4 -*-
    tabwidget.cpp

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

#include "tabwidget.h"
#include "tabwidget_p.h"

#include "models/keylistmodel.h"

#include <KTabWidget>

#include <QGridLayout>
#include <QMap>

#include <cassert>

Page::Page( QAbstractItemModel* model, QWidget * parent )
    : QWidget( parent ),
      m_view( new QTreeView( this ) ),
      m_proxy( new Kleo::KeyListSortFilterProxyModel( this ) )
{
    assert( model );
    m_proxy->setSourceModel( model );
    m_view->setRootIsDecorated( false );
    m_view->setSortingEnabled( true );
    m_view->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    m_view->setModel( m_proxy );
}

void Page::setFilter( const QString& str )
{
    if ( str == m_filter )
        return;
    m_filter = str;
    m_proxy->setFilterFixedString( m_filter ); 
}

QAbstractItemView* Page::view() const
{
    return m_view;
}

Page::~Page() {}

class TabWidget::Private {
    friend class ::TabWidget;
    TabWidget * const q;

public:
    explicit Private( TabWidget * qq );
    ~Private() {};

    QMap<QAbstractItemView*, boost::shared_ptr<Page> > pages;
    KTabWidget * tabWidget;
    void currentIndexChanged( int index );
};

TabWidget::Private::Private( TabWidget* qq ) : q( qq )
{
    QGridLayout * const layout = new QGridLayout( q );
    tabWidget = new KTabWidget;
    connect( tabWidget, SIGNAL( currentChanged( int ) ),
             q, SLOT( currentIndexChanged( int ) ) );
    layout->addWidget( tabWidget, 0, 0 );
}

void TabWidget::Private::Private::currentIndexChanged( int index )
{
    QAbstractItemView* const view = qobject_cast<QAbstractItemView*>( tabWidget->widget( index ) );
    assert( view );
    emit q->currentViewChanged( view );
}

TabWidget::TabWidget( QWidget * parent, Qt::WindowFlags flags ) : QWidget( parent, flags ), d( new Private( this ) )
{
}

TabWidget::~TabWidget()
{
}

void TabWidget::setFilter( const QString& str )
{
    QAbstractItemView * const view = currentView();
    assert( view || d->tabWidget->count() == 0 );
    if ( !view )
        return;
    assert( d->pages.contains( view ) );
    d->pages[view]->setFilter( str );
}

QAbstractItemView * TabWidget::currentView() const
{
    return qobject_cast<QAbstractItemView*>( d->tabWidget->currentWidget() );
    
}

QAbstractItemView * TabWidget::addView( Kleo::AbstractKeyListModel * model, const QString& caption )
{
    const boost::shared_ptr<Page> page( new Page( model ) );
    d->tabWidget->addTab( page->view(), caption );
    d->pages[page->view()] = page;
    return page->view();
}

#include "moc_tabwidget.cpp"
#include "moc_tabwidget_p.cpp"
