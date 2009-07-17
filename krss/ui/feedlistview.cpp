/*
    Copyright (C) 2008       Dmitry Ivanov <vonami@gmail.com>
                  2007, 2009 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "feedlistview.h"
#include "feedlistdelegate.h"
#include "krss/feedlistmodel.h"
#include "krss/treenode.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMenu>

#include <QByteArray>
#include <QHeaderView>
#include <QMetaType>
#include <QPointer>

#include <cassert>

using namespace KRss;
using boost::shared_ptr;

// helper functions
static QModelIndex prevIndex( const QModelIndex&idx )
{
    if ( !idx.isValid() )
        return QModelIndex();
    const QAbstractItemModel* const model = idx.model();
    assert( model );

    if ( idx.row() > 0 ) {
        QModelIndex i = idx.sibling( idx.row() - 1, idx.column() );
        while ( model->hasChildren( i ) ) {
            i = i.child( model->rowCount( i ) - 1, i.column() );
        }
        return i;
    }
    else
        return idx.parent();
}

static QModelIndex prevFeedIndex( const QModelIndex& idx, bool allowPassed = false )
{
    QModelIndex prev = allowPassed ? idx : prevIndex( idx );
    while ( prev.isValid() && prev.data( FeedListModel::IsTagRole ).toBool() ) {
        prev = prevIndex( prev );
    }
    return prev;
}

static QModelIndex prevUnreadFeedIndex( const QModelIndex& idx, bool allowPassed = false )
{
    QModelIndex prev = allowPassed ? idx : prevIndex( idx );
    while ( prev.isValid() && ( prev.data( FeedListModel::IsTagRole ).toBool() ||
            prev.sibling( prev.row(), FeedListModel::UnreadCountColumn ).data().toInt() == 0 ) ) {
        prev = prevIndex( prev );
    }
    return prev;
}

static QModelIndex lastLeaveChild( const QAbstractItemModel* const model )
{
    assert( model );
    if ( model->rowCount() == 0 )
        return QModelIndex();
    QModelIndex idx = model->index( model->rowCount() - 1, 0 );
    while ( model->hasChildren( idx ) ) {
        idx = idx.child( model->rowCount( idx ) - 1, idx.column() );
    }
    return idx;
}

static QModelIndex nextIndex( const QModelIndex& idx )
{
    if ( !idx.isValid() )
        return QModelIndex();
    const QAbstractItemModel* const model = idx.model();
    assert( model );
    if ( model->hasChildren( idx ) )
        return idx.child( 0, idx.column() );
    QModelIndex i = idx;
    while ( true ) {
        if ( !i.isValid() )
            return i;
        const int siblings = model->rowCount( i.parent() );
        if ( i.row() + 1 < siblings )
            return i.sibling( i.row() + 1, i.column() );
        i = i.parent();
    }
}

static QModelIndex nextFeedIndex( const QModelIndex& idx )
{
    QModelIndex next = nextIndex( idx );
    while ( next.isValid() && next.data( FeedListModel::IsTagRole ).toBool() ) {
        next = nextIndex( next );
    }
    return next;
}

static QModelIndex nextUnreadFeedIndex( const QModelIndex& idx )
{
    QModelIndex next = nextIndex( idx );
    while ( next.isValid() && ( next.data( FeedListModel::IsTagRole ).toBool() ||
            next.sibling( next.row(), FeedListModel::UnreadCountColumn ).data().toInt() == 0 ) ) {
        next = nextIndex( next );
    }
    return next;
}

class KRss::FeedListView::Private
{
    FeedListView* const q;
public:

    explicit Private( FeedListView* );
    ~Private() {
        saveHeaderSettings();
    }

    QByteArray headerState;
    KConfigGroup configGroup;

    void loadHeaderSettings();
    void saveHeaderSettings();
    void slotClicked( const QModelIndex& index );
    void slotActivated( const QModelIndex& index );
    void showHeaderMenu( const QPoint& );
    void headerMenuItemTriggered( QAction* action );

};

FeedListView::Private::Private( FeedListView* qq ) : q( qq )
{
}

void FeedListView::Private::slotClicked( const QModelIndex &index )
{
    emit q->clicked( q->model()->data( index, FeedListModel::TreeNodeRole ).value<shared_ptr<TreeNode> >() );
}

void FeedListView::Private::slotActivated( const QModelIndex &index )
{
    emit q->clicked( q->model()->data( index, FeedListModel::TreeNodeRole ).value<shared_ptr<TreeNode> >() );
}

void FeedListView::Private::showHeaderMenu( const QPoint& pos )
{
    if( ! q->model() )
        return;

    QPointer<KMenu> menu = new KMenu( q );
    menu->addTitle( i18n( "Columns" ) );
    menu->setAttribute( Qt::WA_DeleteOnClose );
    q->connect(menu, SIGNAL(triggered(QAction*)), q, SLOT(headerMenuItemTriggered(QAction*)) );

    for (int i = 0; i < q->model()->columnCount(); ++i)
    {
        const QString col = q->model()->headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();
        QAction* act = menu->addAction( col );
        act->setCheckable( true );
        act->setChecked( !q->header()->isSectionHidden( i ) );
        act->setData( i );
    }

    menu->popup( q->header()->mapToGlobal( pos ) );
}

void FeedListView::Private::loadHeaderSettings()
{
    if ( !configGroup.isValid() )
        return;
    headerState = QByteArray::fromBase64( configGroup.readEntry( "FeedListHeaders" ).toAscii() );
    q->header()->restoreState( headerState );        // needed, even with Qt 4.5
}

void FeedListView::Private::saveHeaderSettings()
{
    if ( q->model() )
        headerState = q->header()->saveState();
    if ( configGroup.isValid() )
        configGroup.writeEntry( "FeedListHeaders", headerState.toBase64() );
}

FeedListView::FeedListView( QWidget *parent )
    :  QTreeView( parent ), d( new Private( this ) )
{
    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    setAlternatingRowColors( true );
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode( QAbstractItemView::DragDrop );
    setDropIndicatorShown( true );
    setAcceptDrops( true );
    setUniformRowHeights( true );
    setItemDelegate( new FeedListDelegate( this ) );
    connect( header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showHeaderMenu(QPoint)) );
    connect( this, SIGNAL(clicked(QModelIndex)),
             this, SLOT(slotClicked(QModelIndex)) );
    connect( this, SIGNAL(activated(QModelIndex)),
             this, SLOT( slotActivated(QModelIndex)) );

    d->loadHeaderSettings();
}

KConfigGroup FeedListView::configGroup() const {
    return d->configGroup;
}

void FeedListView::setConfigGroup( const KConfigGroup& g ) {
    d->configGroup = g;
    d->loadHeaderSettings();
}

FeedListView::~FeedListView()
{
    delete d;
}

void FeedListView::setModel( QAbstractItemModel* m )
{
    if ( model() )
        d->headerState = header()->saveState();

    QTreeView::setModel( m );

    if ( m )
        header()->restoreState( d->headerState );

#ifdef KRSS_PORT_DISABLED
    QStack<QModelIndex> stack;
    stack.push( rootIndex() );
    while ( !stack.isEmpty() )
    {
        const QModelIndex i = stack.pop();
        const int childCount = m->rowCount( i );
        for ( int j = 0; j < childCount; ++j )
        {
            const QModelIndex child = m->index( j, 0, i );
            if ( child.isValid() )
                stack.push( child );
        }
        setExpanded( i, i.data( Akregator::SubscriptionListModel::IsOpenRole ).toBool() );
    }
#endif // KRSS_PORT_DISABLED

    header()->setContextMenuPolicy( Qt::CustomContextMenu );
}

void FeedListView::Private::headerMenuItemTriggered( QAction* action )
{
    assert( action );
    const int col = action->data().toInt();
    if ( action->isChecked() )
        q->header()->showSection( col );
    else
        q->header()->hideSection( col );
}


void FeedListView::slotPrevFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevFeedIndex( current );
    if ( !prev.isValid() )
        prev = prevFeedIndex( lastLeaveChild( model() ), true );
    if ( prev.isValid() )
        setCurrentIndex( prev );
}

void FeedListView::slotNextFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex next = nextFeedIndex( current );
    if ( !next.isValid() )
        next = nextFeedIndex( model()->index( 0, 0 ) );
    if ( next.isValid() )
        setCurrentIndex( next );
}

void FeedListView::slotPrevUnreadFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex prev = prevUnreadFeedIndex( current );
    if ( !prev.isValid() )
        prev = prevUnreadFeedIndex( lastLeaveChild( model() ), true );
    if ( prev.isValid() )
        setCurrentIndex( prev );
}

void FeedListView::slotNextUnreadFeed()
{
    if ( !model() )
        return;
    const QModelIndex current = currentIndex();
    QModelIndex next = nextUnreadFeedIndex( current );
    if ( !next.isValid() )
        next = nextUnreadFeedIndex( model()->index( 0, 0 ) );
    if ( next.isValid() )
        setCurrentIndex( next );
}

#include "feedlistview.moc"
