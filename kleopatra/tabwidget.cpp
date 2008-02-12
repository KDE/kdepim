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

#include <config-kleopatra.h>

#include "tabwidget.h"

#include "action_data.h"

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <kleo/keyfilter.h>
#include <kleo/keyfiltermanager.h>

#include <KLocale>
#include <KTabWidget>
#include <KConfigGroup>
#include <KAction>
#include <KActionCollection>

#include <QGridLayout>
#include <QTimer>
#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QInputDialog>

#include <map>
#include <cassert>

using namespace Kleo;
using namespace boost;

namespace {
class Page : public QWidget {
    Q_OBJECT
    Page( const Page & other );
public:
    Page( const QString & title, const QString & id, const QString & text, QWidget * parent=0 );
    Page( const KConfigGroup & group, QWidget * parent=0 );
    ~Page();

    QAbstractItemView * view() const { return m_view; }

    AbstractKeyListModel * model() const { return m_model; }
    void setModel( AbstractKeyListModel * model );

    QString stringFilter() const { return m_stringFilter; }
    void setStringFilter( const QString & filter ); 

    const shared_ptr<KeyFilter> & keyFilter() const { return m_keyFilter; }
    void setKeyFilter( const shared_ptr<KeyFilter> & filter );

    QString title() const { return m_title.isEmpty() && m_keyFilter ? m_keyFilter->name() : m_title ; }
    void setTitle( const QString & title );

    bool canBeClosed() const { return m_canBeClosed; }
    bool canBeRenamed() const { return m_canBeRenamed; }
    bool canChangeStringFilter() const { return m_canChangeStringFilter; }
    bool canChangeKeyFilter() const { return m_canChangeKeyFilter; }

    void saveTo( KConfigGroup & group ) const;

    Page * clone() const { return new Page( *this ); }

    void liftAllRestrictions() {
        m_canBeClosed = m_canBeRenamed = m_canChangeStringFilter = m_canChangeKeyFilter = true;
    }

Q_SIGNALS:
    void titleChanged( const QString & title );
    void stringFilterChanged( const QString & filter );
    void keyFilterChanged( const boost::shared_ptr<Kleo::KeyFilter> & filter );

protected:
    void resizeEvent( QResizeEvent * e ) {
        QWidget::resizeEvent( e );
        m_view->resize( e->size() );
    }

private:
    void init();

private:
    KeyListSortFilterProxyModel m_proxy;
    QTreeView * m_view;
    AbstractKeyListModel * m_model;

    QString m_stringFilter;
    shared_ptr<KeyFilter> m_keyFilter;
    QString m_title;
    bool m_canBeClosed : 1;
    bool m_canBeRenamed : 1;
    bool m_canChangeStringFilter : 1;
    bool m_canChangeKeyFilter : 1;
};
} // anon namespace

Page::Page( const Page & other )
    : QWidget( 0 ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_model( other.m_model ),
      m_stringFilter( other.m_stringFilter ),
      m_keyFilter( other.m_keyFilter ),
      m_title( other.m_title ),
      m_canBeClosed( other.m_canBeClosed ),
      m_canBeRenamed( other.m_canBeRenamed ),
      m_canChangeStringFilter( other.m_canChangeStringFilter ),
      m_canChangeKeyFilter( other.m_canChangeKeyFilter )
{
    init();
}

Page::Page( const QString & title, const QString & id, const QString & text, QWidget * parent )
    : QWidget( parent ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_model( 0 ),
      m_stringFilter( text ),
      m_keyFilter( KeyFilterManager::instance()->keyFilterByID( id ) ),
      m_title( title ),
      m_canBeClosed( true ),
      m_canBeRenamed( true ),
      m_canChangeStringFilter( true ),
      m_canChangeKeyFilter( true )
{
    init();
}

static const char TITLE_ENTRY[] = "title";
static const char STRING_FILTER_ENTRY[] = "string-filter";
static const char KEY_FILTER_ENTRY[] = "key-filter";

Page::Page( const KConfigGroup & group, QWidget * parent )
    : QWidget( parent ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_model( 0 ),
      m_stringFilter( group.readEntry( STRING_FILTER_ENTRY ) ),
      m_keyFilter( KeyFilterManager::instance()->keyFilterByID( group.readEntry( KEY_FILTER_ENTRY ) ) ),
      m_title( group.readEntry( TITLE_ENTRY ) ),
      m_canBeClosed( !group.isImmutable() ),
      m_canBeRenamed( !group.isEntryImmutable( TITLE_ENTRY ) ),
      m_canChangeStringFilter( !group.isEntryImmutable( STRING_FILTER_ENTRY ) ),
      m_canChangeKeyFilter( !group.isEntryImmutable( KEY_FILTER_ENTRY ) )
{
    init();
}

void Page::init() {
    KDAB_SET_OBJECT_NAME( m_proxy );
    KDAB_SET_OBJECT_NAME( m_view );

    if ( m_model )
        m_proxy.setSourceModel( m_model );
    m_proxy.setFilterFixedString( m_stringFilter );
    m_proxy.setKeyFilter( m_keyFilter );
    m_view->setModel( &m_proxy );
}

Page::~Page() {}

void Page::saveTo( KConfigGroup & group ) const {

    group.writeEntry( TITLE_ENTRY, m_title );
    group.writeEntry( STRING_FILTER_ENTRY, m_stringFilter );
    group.writeEntry( KEY_FILTER_ENTRY, m_keyFilter ? m_keyFilter->id() : QString() );

}

void Page::setModel( AbstractKeyListModel * model ) {
    if ( model == m_model )
        return;
    m_model = model;
    m_proxy.setSourceModel( model );
}

void Page::setStringFilter( const QString & filter ) {
    if ( filter == m_stringFilter )
        return;
    if ( !m_canChangeStringFilter )
        return;
    m_stringFilter = filter;
    m_proxy.setFilterFixedString( filter ); 
    emit stringFilterChanged( filter );
}

void Page::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    if ( filter == m_keyFilter || filter && m_keyFilter && filter->id() == m_keyFilter->id() )
        return;
    if ( !m_canChangeKeyFilter )
        return;
    const QString oldTitle = title();
    m_keyFilter = filter;
    m_proxy.setKeyFilter( filter );
    const QString newTitle = title();
    emit keyFilterChanged( filter );
    if ( oldTitle != newTitle )
        emit titleChanged( newTitle );
}

void Page::setTitle( const QString & t ) {
    if ( t == m_title )
        return;
    if ( !m_canBeRenamed )
        return;
    const QString oldTitle = title();
    m_title = t;
    const QString newTitle = title();
    if ( oldTitle != newTitle )
        emit titleChanged( newTitle );
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
    void slotContextMenu( const QPoint & p ) {
        slotContextMenu( 0, p );
    }
    void slotContextMenu( QWidget * w, const QPoint & p );
    void currentIndexChanged( int index );
    void slotPageTitleChanged( const QString & title );
    void slotPageKeyFilterChanged( const shared_ptr<KeyFilter> & filter );
    void slotPageStringFilterChanged( const QString & filter );
    void slotRenameCurrentTab() {
        renamePage( currentPage() );
    }
    void slotNewTab();
    void slotDuplicateCurrentTab() {
        duplicatePage( currentPage() );
    }
    void slotCloseCurrentTab() {
        closePage( currentPage() );
    }
    void slotMoveCurrentTabLeft() {
        movePageLeft( currentPage() );
    }
    void slotMoveCurrentTabRight() {
        movePageRight( currentPage() );
    }

    void renamePage( Page * page );
    void duplicatePage( Page * page );
    void closePage( Page * page );
    void movePageLeft( Page * page );
    void movePageRight( Page * page );

    void enableDisableCurrentPageActions();
    void enableDisablePageActions( QAction * actions[], const Page * page );

    Page * currentPage() const {
        assert( !tabWidget.currentWidget() || qobject_cast<Page*>( tabWidget.currentWidget() ) );
        return static_cast<Page*>( tabWidget.currentWidget() );
    }
    Page * page( unsigned int idx ) const {
        assert( !tabWidget.widget( idx ) || qobject_cast<Page*>( tabWidget.widget( idx ) ) );
        return static_cast<Page*>( tabWidget.widget( idx ) );
    }

    Page * senderPage() const {
        QObject * const sender = q->sender();
        assert( !sender || qobject_cast<Page*>( sender ) );
        return static_cast<Page*>( sender );
    }

    bool isSenderCurrentPage() const {
        Page * const sp = senderPage();
        return sp && sp == currentPage();
    }

    QAbstractItemView * addView( Page * page );
    void setCornerAction( QAction * action, Qt::Corner corner );

private:
    AbstractKeyListModel * model;
    KTabWidget tabWidget;
    enum { Rename, Duplicate, Close, MoveLeft, MoveRight, NumPageActions };
    QAction * newAction;
    QAction * currentPageActions[NumPageActions];
    QAction * otherPageActions[NumPageActions];
};

TabWidget::Private::Private( TabWidget * qq )
    : q( qq ),
      model( 0 ),
      tabWidget( q )
{
    KDAB_SET_OBJECT_NAME( tabWidget );

    tabWidget.setTabBarHidden( true );
    tabWidget.setTabReorderingEnabled( true );

    connect( &tabWidget, SIGNAL(currentChanged(int)),
             q, SLOT(currentIndexChanged(int)) );
    connect( &tabWidget, SIGNAL(contextMenu(QPoint)),
             q, SLOT(slotContextMenu(QPoint)) );
    connect( &tabWidget, SIGNAL(contextMenu(QWidget*,QPoint)),
             q, SLOT(slotContextMenu(QWidget*,QPoint)) );

    const action_data actionDataNew = {
        "window_new_tab", i18n("New Tab"), i18n("Open a new tab"),
        "tab-new", q, SLOT(slotNewTab()), i18n("CTRL+SHIFT+N"), false, true
    };

    newAction = make_action_from_data( actionDataNew, q );

    struct action_data actionData[NumPageActions] = {
        { "window_rename_tab", i18n("Rename Tab..."), i18n("Rename this tab"),
          "edit-rename", q, SLOT(slotRenameCurrentTab()), i18n("CTRL+SHIFT+R"), false, false },
        { "window_duplicate_tab", i18n("Duplicate Current Tab"), i18n("Duplicate the current tab"),
          "tab-duplicate", q, SLOT(slotDuplicateCurrentTab()), i18n("CTRL+SHIFT+D"), false, true },
        { "window_close_tab", i18n("Close Current Tab"), i18n("Close the current tab"),
          "tab-close", q, SLOT(slotCloseCurrentTab()), i18n("CTRL+SHIFT+W"), false, false }, // ### CTRL-W when available
        { "window_move_tab_left", i18n("Move Tab Left"), QString(),
          0, q, SLOT(slotMoveCurrentTabLeft()), i18n("CTRL+SHIFT+LEFT"), false, false },
        { "window_move_tab_right", i18n("Move Tab Right"), QString(),
          0, q, SLOT(slotMoveCurrentTabRight()), i18n("CTRL+SHIFT+RIGHT"), false, false },
    };

    for ( unsigned int i = 0 ; i < NumPageActions ; ++i )
        currentPageActions[i] = make_action_from_data( actionData[i], q );

    for ( unsigned int i = 0 ; i < NumPageActions ; ++i ) {
        action_data ad = actionData[i];
        assert( QString::fromLatin1( ad.name ).startsWith( "window_" ) );
        ad.name = ad.name + strlen("window_");
        ad.tooltip.clear();
        ad.receiver = 0;
        ad.shortcut.clear();
        otherPageActions[i] = make_action_from_data( ad, q );
    }

    setCornerAction( newAction,                 Qt::TopLeftCorner  );
    setCornerAction( currentPageActions[Close], Qt::TopRightCorner );
}

void TabWidget::Private::slotContextMenu( QWidget * w, const QPoint & p ) {
    assert( !w || qobject_cast<Page*>( w ) );
    Page * const contextMenuPage = static_cast<Page*>( w );
    const Page * const current = currentPage();
    
    QAction ** const actions = contextMenuPage == current ? currentPageActions : otherPageActions ;

    if ( contextMenuPage != current )
        enableDisablePageActions( actions, contextMenuPage );

    QMenu menu;
    menu.addAction( actions[Rename] );
    menu.addSeparator();
    menu.addAction( newAction );
    menu.addAction( actions[Duplicate] );
    menu.addSeparator();
    menu.addAction( actions[MoveLeft] );
    menu.addAction( actions[MoveRight] );
    menu.addSeparator();
    menu.addAction( actions[Close] );

    const QAction * const action = menu.exec( p );

    if ( contextMenuPage == current || action == newAction )
        return; // performed through signal/slot connections...

    if ( action == otherPageActions[Rename] )
        renamePage( contextMenuPage );
    else if ( action == otherPageActions[Duplicate] )
        duplicatePage( contextMenuPage );
    else if ( action == otherPageActions[Close] )
        closePage( contextMenuPage );
    else if ( action == otherPageActions[MoveLeft] )
        movePageLeft( contextMenuPage );
    else if ( action == otherPageActions[MoveRight] )
        movePageRight( contextMenuPage );
        
}

void TabWidget::Private::currentIndexChanged( int index ) {
    const Page * const page = this->page( index );
    emit q->currentViewChanged( page ? page->view() : 0 );
    emit q->keyFilterChanged( page ? page->keyFilter() : shared_ptr<KeyFilter>() );
    emit q->stringFilterChanged( page ? page->stringFilter() : QString() );
    enableDisableCurrentPageActions();
}

void TabWidget::Private::enableDisableCurrentPageActions() {
    const Page * const page = currentPage();

    emit q->enableChangeStringFilter( page && page->canChangeStringFilter() );
    emit q->enableChangeKeyFilter( page && page->canChangeKeyFilter() );

    enableDisablePageActions( currentPageActions, page );
}

void TabWidget::Private::enableDisablePageActions( QAction * actions[], const Page * p ) {
    actions[Rename]->setEnabled( p && p->canBeRenamed() );
    actions[Duplicate]->setEnabled( p );
    actions[Close]->setEnabled( p && p->canBeClosed() && tabWidget.count() > 1 );
    actions[MoveLeft] ->setEnabled( p && tabWidget.indexOf( const_cast<Page*>(p) ) != 0 );
    actions[MoveRight]->setEnabled( p && tabWidget.indexOf( const_cast<Page*>(p) ) != tabWidget.count()-1 );
}

void TabWidget::Private::slotPageTitleChanged( const QString & ) {
    if ( Page * const page = senderPage() )
        tabWidget.setTabText( tabWidget.indexOf( page ), page->title() );
}

void TabWidget::Private::slotPageKeyFilterChanged( const shared_ptr<KeyFilter> & kf ) {
    if ( isSenderCurrentPage() )
        emit q->keyFilterChanged( kf );
}

void TabWidget::Private::slotPageStringFilterChanged( const QString & filter ) {
    if ( isSenderCurrentPage() )
        emit q->stringFilterChanged( filter );
}

void TabWidget::Private::slotNewTab() {
    q->addView( QString(), "my-certificates" );
    tabWidget.setCurrentIndex( tabWidget.count()-1 );
}

void TabWidget::Private::renamePage( Page * page ) {
    if ( !page )
        return;
    bool ok = false;
    const QString text = QInputDialog::getText( q, i18n("Rename Tab"), i18n("New tab title:"), QLineEdit::Normal, page->title(), &ok );
    if ( !ok )
        return;
    page->setTitle( text );
}

void TabWidget::Private::duplicatePage( Page * page ) {
    if ( !page )
        return;
    Page * const clone = page->clone();
    assert( clone );
    clone->liftAllRestrictions();
    addView( clone );
}

void TabWidget::Private::closePage( Page * page) {
    if ( !page || !page->canBeClosed() || tabWidget.count() <= 1 )
        return;
    tabWidget.removeTab( tabWidget.indexOf( page ) );
    enableDisableCurrentPageActions();
}

void TabWidget::Private::movePageLeft( Page * page ) {
    if ( !page )
        return;
    const int idx = tabWidget.indexOf( page );
    if ( idx <= 0 )
        return;
    tabWidget.moveTab( idx, idx-1 );
    enableDisableCurrentPageActions();
}

void TabWidget::Private::movePageRight( Page * page ) {
    if ( !page )
        return;
    const int idx = tabWidget.indexOf( page );
    if ( idx < 0 || idx >= tabWidget.count()-1 )
        return;
    tabWidget.moveTab( idx, idx+1 );
    enableDisableCurrentPageActions();
}

TabWidget::TabWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{

}

TabWidget::~TabWidget() {}

void TabWidget::setModel( AbstractKeyListModel * model ) {
    if ( model == d->model )
        return;
    d->model = model;
    for ( unsigned int i = 0, end = count() ; i != end ; ++i )
        if ( Page * const page = d->page( i ) )
            page->setModel( model );
}

void TabWidget::Private::setCornerAction( QAction * action, Qt::Corner corner ) {
    if ( !action )
        return;
    QToolButton * b = new QToolButton;
    b->setDefaultAction( action );
    tabWidget.setCornerWidget( b, corner );
}

void TabWidget::setStringFilter( const QString & filter ) {
    if ( Page * const page = d->currentPage() )
        page->setStringFilter( filter );
}

void TabWidget::setKeyFilter( const shared_ptr<KeyFilter> & filter ) {
    if ( Page * const page = d->currentPage() )
        page->setKeyFilter( filter );
}

QAbstractItemView * TabWidget::currentView() const {
    if ( Page * const page = d->currentPage() )
        return page->view();
    else
        return 0;
}

unsigned int TabWidget::count() const {
    return d->tabWidget.count();
}

void TabWidget::createActions( KActionCollection * coll ) {
    if ( !coll )
        return;
    coll->addAction( d->newAction->objectName(), d->newAction );
    for ( unsigned int i = 0 ; i < Private::NumPageActions ; ++i ) {
        QAction * a = d->currentPageActions[i];
        coll->addAction( a->objectName(), a );
    }
}

void TabWidget::resizeEvent( QResizeEvent * e ) {
    QWidget::resizeEvent( e );
    d->tabWidget.resize( e->size() );
}

QAbstractItemView * TabWidget::addView( const QString & title, const QString & id, const QString & text ) {
    return d->addView( new Page( title, id, text ) );
}

QAbstractItemView * TabWidget::addView( const KConfigGroup & group ) {
    return d->addView( new Page( group ) );
}

QAbstractItemView * TabWidget::Private::addView( Page * page ) {
    if ( !page )
        return 0;

    page->setModel( model );

    connect( page, SIGNAL(titleChanged(QString)),
             q, SLOT(slotPageTitleChanged(QString)) );
    connect( page, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
             q, SLOT(slotPageKeyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)) );
    connect( page, SIGNAL(stringFilterChanged(QString)),
             q, SLOT(slotPageStringFilterChanged(QString)) );

    QAbstractItemView * const previous = q->currentView(); 
    tabWidget.addTab( page, page->title() );
    tabWidget.setTabBarHidden( tabWidget.count() < 2 );
    // work around a bug in QTabWidget (tested with 4.3.2) not emitting currentChanged() when the first widget is inserted
    QAbstractItemView * const current = q->currentView(); 
    if ( previous != current )
        currentIndexChanged( tabWidget.currentIndex() );
    enableDisableCurrentPageActions();
    return page->view();
}

void TabWidget::saveTab( unsigned int idx, KConfigGroup & group ) const {
    if ( const Page * const page = d->page( idx ) )
        page->saveTo( group );
}

#include "moc_tabwidget.cpp"
#include "tabwidget.moc"
