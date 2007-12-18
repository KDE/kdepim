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

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <kleo/keyfilter.h>

#include <KTabWidget>

#include <QGridLayout>
#include <QTimer>
#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <map>
#include <cassert>

using namespace Kleo;
using namespace boost;

namespace {
class Page : public QWidget {
    Q_OBJECT
public:
    explicit Page( QAbstractItemModel * sourceModel, QWidget * parent=0 );

    QAbstractItemView * view() const { return m_view; }

    QString stringFilter() const { return m_stringFilter; }
    void setStringFilter( const QString & filter ); 

    const shared_ptr<KeyFilter> & keyFilter() const { return m_keyFilter; }
    void setKeyFilter( const shared_ptr<KeyFilter> & filter );

protected:
    void resizeEvent( QResizeEvent * e ) {
        QWidget::resizeEvent( e );
        m_view->resize( e->size() );
    }

private:
    KeyListSortFilterProxyModel m_proxy;
    QTreeView * m_view;
    QString m_stringFilter;
    shared_ptr<KeyFilter> m_keyFilter;
};
} // anon namespace

Page::Page( QAbstractItemModel * model, QWidget * parent )
    : QWidget( parent ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_stringFilter(),
      m_keyFilter()
{
    KDAB_SET_OBJECT_NAME( m_proxy );
    KDAB_SET_OBJECT_NAME( m_view );

    assert( model );
    m_proxy.setSourceModel( model );
#if 0 // done by controller, when registering view
    m_view->setRootIsDecorated( false );
    m_view->setSortingEnabled( true );
    m_view->sortByColumn( AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
#endif
    m_view->setModel( &m_proxy );
}

void Page::setStringFilter( const QString & filter ) {
    if ( filter == m_stringFilter )
        return;
    m_stringFilter = filter;
    m_proxy.setFilterFixedString( filter ); 
}

void Page::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    if ( filter == m_keyFilter || filter && m_keyFilter && filter->id() == m_keyFilter->id() )
        return;
    m_keyFilter = filter;
    m_proxy.setKeyFilter( filter );
}

//
//
// TabWidget
//
//

class TabWidget::Private {
    friend class ::TabWidget;
    TabWidget * const q;
public:
    explicit Private( TabWidget * qq );
    ~Private() {};

private:
    void currentIndexChanged( int index );

    const shared_ptr<Page> & currentPage() const;

private:
    std::map<QAbstractItemView*, shared_ptr<Page> > pages;
    KTabWidget * tabWidget;
};

TabWidget::Private::Private( TabWidget* qq ) : q( qq )
{
    QGridLayout * const layout = new QGridLayout( q );
    tabWidget = new KTabWidget;
    connect( tabWidget, SIGNAL( currentChanged( int ) ),
             q, SLOT( currentIndexChanged( int ) ) );
    layout->addWidget( tabWidget, 0, 0 );
}

void TabWidget::Private::currentIndexChanged( int index )
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

void TabWidget::setStringFilter( const QString & filter ) {
    if ( const shared_ptr<Page> & page = d->currentPage() )
        page->setStringFilter( filter );
}

void TabWidget::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    if ( const shared_ptr<Page> & page = d->currentPage() )
        page->setKeyFilter( filter );
}

QAbstractItemView * TabWidget::currentView() const
{
    return qobject_cast<QAbstractItemView*>( d->tabWidget->currentWidget() );
}

const shared_ptr<Page> & TabWidget::Private::currentPage() const {
    QAbstractItemView * const view = q->currentView();
    assert( view || !tabWidget->count() );
    if ( !view ) {
        static const shared_ptr<Page> empty;
        return empty;
    }
    const std::map<QAbstractItemView*,shared_ptr<Page> >::const_iterator it
        = pages.find( view );
    assert( it != pages.end() );
    return it->second;
}

QAbstractItemView * TabWidget::addView( AbstractKeyListModel * model, const QString& caption )
{
    QAbstractItemView * const previous = currentView(); 
    const shared_ptr<Page> page( new Page( model ) );
    d->pages[page->view()] = page;
    d->tabWidget->addTab( page->view(), caption );
    // work around a bug in QTabWidget (tested with 4.3.2) not emitting currentChanged() when the first widget is inserted
    QAbstractItemView * const current = currentView(); 
    if ( previous != current )
        emit currentViewChanged( current ); 
    return page->view();
}

#include "moc_tabwidget.cpp"
#include "tabwidget.moc"
