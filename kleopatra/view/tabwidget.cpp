/* -*- mode: c++; c-basic-offset:4 -*-
    view/tabwidget.cpp

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

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <utils/action_data.h>
#include <utils/headerview.h>
#include <utils/stl_util.h>

#include <kleo/keyfilter.h>
#include <kleo/keyfiltermanager.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KTabWidget>
#include <KConfigGroup>
#include <KConfig>
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
#include <QItemSelectionModel>
#include <QItemSelection>

#include <map>
#include <vector>
#include <cassert>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace {
class Page : public QWidget {
    Q_OBJECT
    Page( const Page & other );
public:
    Page( const QString & title, const QString & id, const QString & text, QWidget * parent=0 );
    Page( const KConfigGroup & group, QWidget * parent=0 );
    ~Page();

    QTreeView * view() const { return m_view; }

    AbstractKeyListModel * model() const {
        return m_isHierarchical ? m_hierarchicalModel : m_flatModel ;
    }
    void setFlatModel( AbstractKeyListModel * model );
    void setHierarchicalModel( AbstractKeyListModel * model );

    void setHierarchical( bool hierarchical );
    bool isHierarchical() const { return m_isHierarchical; }

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
    bool canChangeHierarchical() const { return m_canChangeHierarchical; }

    void saveTo( KConfigGroup & group ) const;

    Page * clone() const { return new Page( *this ); }

    void liftAllRestrictions() {
        m_canBeClosed = m_canBeRenamed = m_canChangeStringFilter = m_canChangeKeyFilter = m_canChangeHierarchical = true;
    }

Q_SIGNALS:
    void titleChanged( const QString & title );
    void stringFilterChanged( const QString & filter );
    void keyFilterChanged( const boost::shared_ptr<Kleo::KeyFilter> & filter );
    void hierarchicalChanged( bool on );

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
    AbstractKeyListModel * m_flatModel;
    AbstractKeyListModel * m_hierarchicalModel;

    QString m_stringFilter;
    shared_ptr<KeyFilter> m_keyFilter;
    QString m_title;
    bool m_isHierarchical : 1;
    bool m_canBeClosed : 1;
    bool m_canBeRenamed : 1;
    bool m_canChangeStringFilter : 1;
    bool m_canChangeKeyFilter : 1;
    bool m_canChangeHierarchical : 1;
};
} // anon namespace

Page::Page( const Page & other )
    : QWidget( 0 ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_flatModel( other.m_flatModel ),
      m_hierarchicalModel( other.m_hierarchicalModel ),
      m_stringFilter( other.m_stringFilter ),
      m_keyFilter( other.m_keyFilter ),
      m_title( other.m_title ),
      m_isHierarchical( other.m_isHierarchical ),
      m_canBeClosed( other.m_canBeClosed ),
      m_canBeRenamed( other.m_canBeRenamed ),
      m_canChangeStringFilter( other.m_canChangeStringFilter ),
      m_canChangeKeyFilter( other.m_canChangeKeyFilter ),
      m_canChangeHierarchical( other.m_canChangeHierarchical )
{
    init();
}

Page::Page( const QString & title, const QString & id, const QString & text, QWidget * parent )
    : QWidget( parent ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_flatModel( 0 ),
      m_hierarchicalModel( 0 ),
      m_stringFilter( text ),
      m_keyFilter( KeyFilterManager::instance()->keyFilterByID( id ) ),
      m_title( title ),
      m_isHierarchical( true ),
      m_canBeClosed( true ),
      m_canBeRenamed( true ),
      m_canChangeStringFilter( true ),
      m_canChangeKeyFilter( true ),
      m_canChangeHierarchical( true )
{
    init();
}

static const char TITLE_ENTRY[] = "title";
static const char STRING_FILTER_ENTRY[] = "string-filter";
static const char KEY_FILTER_ENTRY[] = "key-filter";
static const char HIERARCHICAL_VIEW_ENTRY[] = "hierarchical-view";
static const char COLUMN_SIZES[] = "column-sizes";

Page::Page( const KConfigGroup & group, QWidget * parent )
    : QWidget( parent ),
      m_proxy(),
      m_view( new QTreeView( this ) ),
      m_flatModel( 0 ),
      m_hierarchicalModel( 0 ),
      m_stringFilter( group.readEntry( STRING_FILTER_ENTRY ) ),
      m_keyFilter( KeyFilterManager::instance()->keyFilterByID( group.readEntry( KEY_FILTER_ENTRY ) ) ),
      m_title( group.readEntry( TITLE_ENTRY ) ),
      m_isHierarchical( group.readEntry( HIERARCHICAL_VIEW_ENTRY, true ) ),
      m_canBeClosed( !group.isImmutable() ),
      m_canBeRenamed( !group.isEntryImmutable( TITLE_ENTRY ) ),
      m_canChangeStringFilter( !group.isEntryImmutable( STRING_FILTER_ENTRY ) ),
      m_canChangeKeyFilter( !group.isEntryImmutable( KEY_FILTER_ENTRY ) ),
      m_canChangeHierarchical( !group.isEntryImmutable( HIERARCHICAL_VIEW_ENTRY ) )
{
    init();
    assert( m_view );
    assert( m_view->header() );
    assert( qobject_cast<HeaderView*>( m_view->header() ) == static_cast<HeaderView*>( m_view->header() ) );
    if ( HeaderView * const hv = static_cast<HeaderView*>( m_view->header() ) ) {
        const QList<int> sizes = group.readEntry( COLUMN_SIZES, QList<int>() );
        if ( !sizes.empty() )
            hv->setSectionSizes( kdtools::copy< std::vector<int> >( sizes ) );
    }
}

static const QHeaderView::ResizeMode resize_modes[AbstractKeyListModel::NumColumns] = {
    QHeaderView::Stretch,          // Name
    QHeaderView::Stretch, // EMail
    QHeaderView::Fixed, // Valid From
    QHeaderView::Fixed, // Valid Until
    QHeaderView::Fixed, // Details
    QHeaderView::Fixed, // Fingerprint
};

static void adjust_header( HeaderView * hv ) {
    for ( int i = 0, end = AbstractKeyListModel::NumColumns ; i < end ; ++i )
        hv->setSectionResizeMode( i, resize_modes[i] );
}

void Page::init() {
    KDAB_SET_OBJECT_NAME( m_proxy );
    KDAB_SET_OBJECT_NAME( m_view );

    HeaderView * headerView = new HeaderView( Qt::Horizontal );
    KDAB_SET_OBJECT_NAME( headerView );
    m_view->setHeader( headerView );
    adjust_header( headerView );

    m_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_view->setSelectionMode( QAbstractItemView::ExtendedSelection );
    //m_view->setAlternatingRowColors( true );
    m_view->setAllColumnsShowFocus( true );
    m_view->setSortingEnabled( true );

    if ( model() )
        m_proxy.setSourceModel( model() );
    m_proxy.setFilterFixedString( m_stringFilter );
    m_proxy.setKeyFilter( m_keyFilter );
    m_view->setModel( &m_proxy );
}

Page::~Page() {}

void Page::saveTo( KConfigGroup & group ) const {

    group.writeEntry( TITLE_ENTRY, m_title );
    group.writeEntry( STRING_FILTER_ENTRY, m_stringFilter );
    group.writeEntry( KEY_FILTER_ENTRY, m_keyFilter ? m_keyFilter->id() : QString() );
    group.writeEntry( HIERARCHICAL_VIEW_ENTRY, m_isHierarchical );
    if ( const HeaderView * const hv = m_view ? qobject_cast<HeaderView*>( m_view->header() ) : 0 )
        group.writeEntry( COLUMN_SIZES, kdtools::copy< QList<int> >( hv->sectionSizes() ) );
}

void Page::setFlatModel( AbstractKeyListModel * model ) {
    if ( model == m_flatModel )
        return;
    m_flatModel = model;
    if ( !m_isHierarchical )
        m_proxy.setSourceModel( model );
}

void Page::setHierarchicalModel( AbstractKeyListModel * model ) {
    if ( model == m_hierarchicalModel )
        return;
    m_hierarchicalModel = model;
    if ( m_isHierarchical )
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

static QItemSelection itemSelectionFromKeys( const std::vector<Key> & keys, const KeyListSortFilterProxyModel & proxy ) {
    QItemSelection result;
    Q_FOREACH( const Key & key, keys ) {
        const QModelIndex mi = proxy.index( key );
        if ( mi.isValid() )
            result.merge( QItemSelection( mi, mi ), QItemSelectionModel::Select );
    }
    return result;
}

void Page::setHierarchical( bool on ) {
    if ( on == m_isHierarchical )
        return;
    if ( !m_canChangeHierarchical )
        return;
    const std::vector<Key> selectedKeys = m_proxy.keys( m_view->selectionModel()->selectedRows() );
    const Key currentKey = m_proxy.key( m_view->currentIndex() );
    m_isHierarchical = on;
    m_proxy.setSourceModel( model() );
    m_view->selectionModel()->select( itemSelectionFromKeys( selectedKeys, m_proxy ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    if ( !currentKey.isNull() ) {
        const QModelIndex currentIndex = m_proxy.index( currentKey );
        if ( currentIndex.isValid() ) {
            m_view->selectionModel()->setCurrentIndex( m_proxy.index( currentKey ), QItemSelectionModel::NoUpdate );
            m_view->scrollTo( currentIndex );
        }
    }
    emit hierarchicalChanged( on );
}

//
//
// TabWidget
//
//

class TabWidget::Private {
    friend class ::Kleo::TabWidget;
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
    void slotPageHierarchyChanged( bool on );

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
    void slotToggleHierarchicalView( bool on ) {
        toggleHierarchicalView( currentPage(), on );
    }
    void slotExpandAll() {
        expandAll( currentPage() );
    }
    void slotCollapseAll() {
        collapseAll( currentPage() );
    }

    void renamePage( Page * page );
    void duplicatePage( Page * page );
    void closePage( Page * page );
    void movePageLeft( Page * page );
    void movePageRight( Page * page );
    void toggleHierarchicalView( Page * page, bool on );
    void expandAll( Page * page );
    void collapseAll( Page * page );

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

    QTreeView * addView( Page * page );
    void setCornerAction( QAction * action, Qt::Corner corner );

private:
    AbstractKeyListModel * flatModel;
    AbstractKeyListModel * hierarchicalModel;
    KTabWidget tabWidget;
    enum {
        Rename,
        Duplicate,
        Close,
        MoveLeft,
        MoveRight,
        Hierarchical,
        ExpandAll,
        CollapseAll,

        NumPageActions
    };
    QAction * newAction;
    QAction * currentPageActions[NumPageActions];
    QAction * otherPageActions[NumPageActions];
};

TabWidget::Private::Private( TabWidget * qq )
    : q( qq ),
      flatModel( 0 ),
      hierarchicalModel( 0 ),
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
        { "window_view_hierarchical", i18n("Hierarchical Certificate List"), QString(),
          0, q, SLOT(slotToggleHierarchicalView(bool)), QString(), true, false },
        { "window_expand_all", i18n("Expand All"), QString(),
          0, q, SLOT(slotExpandAll()), i18n("CTRL+."), false, false },
        { "window_collapse_all", i18n("Collapse All"), QString(),
          0, q, SLOT(slotCollapseAll()), i18n("CTRL+,"), false, false },
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
    actions[Rename]      ->setEnabled( p && p->canBeRenamed() );
    actions[Duplicate]   ->setEnabled( p );
    actions[Close]       ->setEnabled( p && p->canBeClosed() && tabWidget.count() > 1 );
    actions[MoveLeft]    ->setEnabled( p && tabWidget.indexOf( const_cast<Page*>(p) ) != 0 );
    actions[MoveRight]   ->setEnabled( p && tabWidget.indexOf( const_cast<Page*>(p) ) != tabWidget.count()-1 );
    actions[Hierarchical]->setEnabled( p && p->canChangeHierarchical() );
    actions[Hierarchical]->setChecked( p && p->isHierarchical() );
    actions[ExpandAll]   ->setEnabled( p && p->isHierarchical() );
    actions[CollapseAll] ->setEnabled( p && p->isHierarchical() );
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

void TabWidget::Private::slotPageHierarchyChanged( bool ) {
    enableDisableCurrentPageActions();
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
    emit q->viewAboutToBeRemoved( page->view() );
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

void TabWidget::Private::toggleHierarchicalView( Page * page, bool on ) {
    if ( !page )
        return;
    page->setHierarchical( on );
}

void TabWidget::Private::expandAll( Page * page ) {
    if ( !page || !page->view() )
        return;
    page->view()->expandAll();
}

void TabWidget::Private::collapseAll( Page * page ) {
    if ( !page || !page->view() )
        return;
    page->view()->collapseAll();
}

TabWidget::TabWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{

}

TabWidget::~TabWidget() {}

void TabWidget::setFlatModel( AbstractKeyListModel * model ) {
    if ( model == d->flatModel )
        return;
    d->flatModel = model;
    for ( unsigned int i = 0, end = count() ; i != end ; ++i )
        if ( Page * const page = d->page( i ) )
            page->setFlatModel( model );
}

AbstractKeyListModel * TabWidget::flatModel() const {
    return d->flatModel;
}

void TabWidget::setHierarchicalModel( AbstractKeyListModel * model ) {
    if ( model == d->hierarchicalModel )
        return;
    d->hierarchicalModel = model;
    for ( unsigned int i = 0, end = count() ; i != end ; ++i )
        if ( Page * const page = d->page( i ) )
            page->setHierarchicalModel( model );
}

AbstractKeyListModel * TabWidget::hierarchicalModel() const {
    return d->hierarchicalModel;
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

void TabWidget::setMultiSelection( bool on ) {
    for ( unsigned int i = 0, end = count() ; i != end ; ++i )
        if ( const Page * const p = d->page( i ) )
            if ( QTreeView * const view = p->view() )
                view->setSelectionMode( on ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection );
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

QTreeView * TabWidget::Private::addView( Page * page ) {
    if ( !page )
        return 0;

    page->setFlatModel( flatModel );
    page->setHierarchicalModel( hierarchicalModel );

    connect( page, SIGNAL(titleChanged(QString)),
             q, SLOT(slotPageTitleChanged(QString)) );
    connect( page, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
             q, SLOT(slotPageKeyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)) );
    connect( page, SIGNAL(stringFilterChanged(QString)),
             q, SLOT(slotPageStringFilterChanged(QString)) );
    connect( page, SIGNAL(hierarchicalChanged(bool)),
             q, SLOT(slotPageHierarchyChanged(bool)) );

    QAbstractItemView * const previous = q->currentView(); 
    tabWidget.addTab( page, page->title() );
    tabWidget.setTabBarHidden( tabWidget.count() < 2 );
    // work around a bug in QTabWidget (tested with 4.3.2) not emitting currentChanged() when the first widget is inserted
    QAbstractItemView * const current = q->currentView(); 
    if ( previous != current )
        currentIndexChanged( tabWidget.currentIndex() );
    enableDisableCurrentPageActions();
    QTreeView * view = page->view();
    emit q->viewAdded( view );
    return view;
}

static QStringList extractViewGroups( const KConfig * config ) {
    return config ? config->groupList().filter( QRegExp( "^View #\\d+$" ) ) : QStringList() ;
}

void TabWidget::loadViews( const KConfig * config ) {
    if ( config )
        Q_FOREACH( const QString & group, extractViewGroups( config ) )
            addView( KConfigGroup( config, group ) );
    if ( !count() ) {
        // add default views:
        addView( QString(), "my-certificates" );
        addView( QString(), "trusted-certificates" );
        addView( QString(), "other-certificates" );
    }
}

void TabWidget::saveViews( KConfig * config ) const {
    if ( !config )
        return;
    Q_FOREACH( QString group, extractViewGroups( config ) )
        config->deleteGroup( group );
    for ( unsigned int i = 0, end = count() ; i != end ; ++i ) {
        if ( const Page * const p = d->page( i ) ) {
            KConfigGroup group( config, QString().sprintf( "View #%u", i ) );
            p->saveTo( group );
        }
    }
}

static void xconnect( const QObject * o1, const char * signal, const QObject * o2, const char * slot ) {
    QObject::connect( o1, signal, o2, slot );
    QObject::connect( o2, signal, o1, slot );
}

void TabWidget::connectSearchBar( QObject * sb ) {
    xconnect( sb, SIGNAL(stringFilterChanged(QString)),
              this, SLOT(setStringFilter(QString)) );
    xconnect( sb, SIGNAL(keyFilterChanged(boost::shared_ptr<Kleo::KeyFilter>)),
              this, SLOT(setKeyFilter(boost::shared_ptr<Kleo::KeyFilter>)) );
    connect( this, SIGNAL(enableChangeStringFilter(bool)),
             sb, SLOT(setChangeStringFilterEnabled(bool)) );
    connect( this, SIGNAL(enableChangeKeyFilter(bool)),
             sb, SLOT(setChangeKeyFilterEnabled(bool)) );
}

#include "moc_tabwidget.cpp"
#include "tabwidget.moc"
