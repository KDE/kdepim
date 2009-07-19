/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "standardactionmanager.h"
#include "feedlist.h"
#include "feedlistmodel.h"
#include "treenode.h"
#include "itemmodel.h"
#include "feed.h"
#include "item.h"
#include "netfeed.h"
#include "netfeedcreatejob.h"
#include "feedjobs.h"
#include "itemjobs.h"
#include "resourcemanager.h"
#include "netresource.h"
#include "ui/netfeedcreatedialog.h"
#include "ui/feedpropertiesdialog.h"
#include "subscriptionsmodel.h"
#include "tagprovider.h"
#include "tagjobs.h"
#include "ui/tagpropertiesdialog.h"

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KLocale>
#include <KInputDialog>
#include <KMessageBox>

#include <QtGui/QItemSelectionModel>
#include <QtGui/QTreeView>

#include <boost/static_assert.hpp>

using namespace KRss;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

static const struct {
    const char *name;
    const char *label;
    const char *icon;
    int shortcut;
    const char* slot;
} actionData[] = {
    { "krss_feed_create", I18N_NOOP( "&New Feed..." ), "feed-subscribe", 0, SLOT( slotCreateNetFeed() ) },
    { "krss_feed_fetch", I18N_NOOP( "&Fetch Feed" ), "view-refresh", Qt::Key_F5, SLOT( slotFetchFeed() ) },
    { "krss_feed_abortfetch", I18N_NOOP( "&Abort Fetch" ), "process-stop", Qt::Key_F8, SLOT( slotAbortFetch() ) },
    { "krss_feed_properties", I18N_NOOP( "Feed &Properties..." ), "document-properties", 0, SLOT( slotFeedProperties() ) },
    { "krss_feed_delete", I18N_NOOP( "&Delete Feed" ), "edit-delete", 0, SLOT( slotDeleteFeed() ) },
    { "krss_tag_create", I18N_NOOP( "&Create tag" ), 0, 0, SLOT( slotCreateTag() ) },
    { "krss_tag_modify", I18N_NOOP( "&Modify tag" ), 0, 0, SLOT( slotModifyTag() ) },
    { "krss_tag_delete", I18N_NOOP( "&Delete tag" ), 0, 0, SLOT( slotDeleteTag() ) },
    { "krss_manage_subscriptions", I18N_NOOP( "&Manage subscriptions" ), 0, 0, SLOT( slotManageSubscriptions() ) },
    { "krss_item_mark_new", I18N_NOOP( "&Mark as new" ), "mail-mark-unread-new", 0, SLOT( slotMarkItemNew() ) },
    { "krss_item_mark_read", I18N_NOOP( "&Mark as read" ), "mail-mark-read", 0, SLOT( slotMarkItemRead() ) },
    { "krss_item_mark_unread", I18N_NOOP( "&Mark as unread" ), "mail-mark-unread", 0, SLOT( slotMarkItemUnread() ) },
    { "krss_item_mark_important", I18N_NOOP( "&Mark as important" ), "mail-mark-important", 0, SLOT( slotMarkItemImportant( bool ) ) },
    { "krss_item_delete", I18N_NOOP( "&Delete item" ), "edit-delete", 0, SLOT( slotDeleteItem() ) }
};
static const int numActionData = sizeof (actionData) / sizeof (*actionData);

BOOST_STATIC_ASSERT( numActionData == StandardActionManager::LastType );


namespace KRss {

class StandardActionManagerPrivate
{
public:

    StandardActionManagerPrivate( KActionCollection *actionCollection, QWidget *parent )
        : m_actionCollection( actionCollection ), m_parentWidget( parent ), m_feedSelectionModel( 0 ),
          m_itemSelectionModel( 0 )
    {
    }

    KActionCollection *m_actionCollection;
    QWidget *m_parentWidget;
    QItemSelectionModel *m_feedSelectionModel;
    QItemSelectionModel *m_itemSelectionModel;
    shared_ptr<const FeedList> m_feedList;
    QString m_subscriptionLabel;
    shared_ptr<const TagProvider> m_tagProvider;
    QVector<KAction*> m_actions;
};

} // namespace KRss


StandardActionManager::StandardActionManager( KActionCollection *actionCollection, QWidget *parent )
    : d( new StandardActionManagerPrivate( actionCollection, parent ) )
{
    d->m_actions.fill( 0, LastType );
}

StandardActionManager::~StandardActionManager()
{
    delete d;
}

void StandardActionManager::setFeedSelectionModel( QItemSelectionModel *feedSelectionModel )
{
    if ( d->m_feedSelectionModel )
        d->m_feedSelectionModel->disconnect( this );

    d->m_feedSelectionModel = feedSelectionModel;
    connect( d->m_feedSelectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( updateActions() ) );

    const FeedListModel * const model = qobject_cast<const FeedListModel*>( d->m_feedSelectionModel->model() );
    Q_ASSERT( model );
    d->m_feedList = model->feedList();
}

void StandardActionManager::setItemSelectionModel( QItemSelectionModel *itemSelectionModel )
{
    if ( d->m_itemSelectionModel )
        d->m_itemSelectionModel->disconnect( this );

    d->m_itemSelectionModel = itemSelectionModel;
    connect( d->m_itemSelectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( updateActions() ) );
}

void StandardActionManager::setSubscriptionLabel( const QString &subscriptionLabel )
{
    d->m_subscriptionLabel = subscriptionLabel;
}

void StandardActionManager::setTagProvider( const shared_ptr<const TagProvider>& provider )
{
    Q_ASSERT( provider );
    d->m_tagProvider = provider;
    updateActions();
}

KAction* StandardActionManager::action( Type type )
{
    Q_ASSERT( type >= 0 && type < LastType );
    Q_ASSERT( actionData[ type ].name );

    if ( d->m_actions[ type ] )
        return d->m_actions[ type ];

    KAction *action = new KAction( d->m_parentWidget );
    if ( actionData[ type ].label )
        action->setText( i18n( actionData[ type ].label ) );

    if ( actionData[ type ].icon )
        action->setIcon( KIcon( QString::fromLatin1( actionData[ type ].icon ) ) );

    action->setShortcut( actionData[ type ].shortcut );

    if ( actionData[ type ].slot )
        connect( action, SIGNAL( triggered( bool ) ), this, actionData[ type ].slot );

    if ( type == MarkItemImportant )
        action->setCheckable( true );

    d->m_actionCollection->addAction( QString::fromLatin1( actionData[ type ].name ), action );
    d->m_actions[ type ] = action;
    updateActions();
    return action;
}

void StandardActionManager::createAllActions()
{
    for ( int i = 0; i < LastType; ++i )
        action( (Type)i );
}

void StandardActionManager::enableAction( Type type, bool enable )
{
    Q_ASSERT( type >= 0 && type < LastType );
    if ( d->m_actions[ type ] )
        d->m_actions[ type ]->setEnabled( enable );
}

void StandardActionManager::updateActions()
{
    enableAction( StandardActionManager::ManageSubscriptions, true );

    // update actions for feeds
    // FIXME: only single selections, better to query the type of the node
    // FIXME: supports only persistent feeds
    enableAction( CreateNetFeed, true );
    bool enableFeedActions = false;
    const QModelIndexList selectedFeeds = d->m_feedSelectionModel ?
                                          d->m_feedSelectionModel->selectedRows() : QModelIndexList();
    if ( !selectedFeeds.isEmpty() ) {
        const shared_ptr<TreeNode> treeNode = selectedFeeds.first().data( FeedListModel::TreeNodeRole).
                                               value<shared_ptr<TreeNode> >();
        if ( treeNode->tier() == TreeNode::FeedTier )
            enableFeedActions = true;
    }
    enableAction( FetchFeed, enableFeedActions );
    enableAction( AbortFetch, enableFeedActions );
    enableAction( FeedProperties, enableFeedActions );
    enableAction( DeleteFeed, enableFeedActions );

    const QModelIndexList selectedItems = d->m_itemSelectionModel ? d->m_itemSelectionModel->selectedRows() : QModelIndexList();

    // update actions for items
    if ( !selectedItems.isEmpty() ) {
        // if there are multiple items selected, enable the MarkItemImportant
        // action only if all the items have the same state of the 'Important' flag
        bool enableImportant = true;
        const bool firstValue = selectedItems.first().data( ItemModel::ItemStatusRole ).value<KRss::Item::Status>() &
                                                           KRss::Item::Important;
        Q_FOREACH( const QModelIndex &selectedIndex, selectedItems ) {
            if ( firstValue != ( selectedIndex.data( ItemModel::ItemStatusRole ).value<KRss::Item::Status>() &
                                                     KRss::Item::Important ) ) {
                enableImportant = false;
                break;
            }
        }
        enableAction( MarkItemNew, true );
        enableAction( MarkItemRead, true );
        enableAction( MarkItemUnread, true );
        enableAction( MarkItemImportant, enableImportant );
        action( MarkItemImportant )->setChecked( enableImportant ? firstValue : false );
        enableAction( DeleteItem, true );
    }
    else {
        enableAction( MarkItemNew, false );
        enableAction( MarkItemRead, false );
        enableAction( MarkItemUnread, false );
        enableAction( MarkItemImportant, false );
        action( MarkItemImportant )->setChecked( false );
        enableAction( DeleteItem, false );
    }

    // update tag-related actions
    if ( !selectedFeeds.isEmpty() ) {
         shared_ptr<TreeNode> treeNode = selectedFeeds.first().data( FeedListModel::TreeNodeRole ).
                                         value<shared_ptr<TreeNode> >();
        if ( treeNode->tier() == TreeNode::TagTier ) {
            enableAction( CreateTag, true );
            enableAction( ModifyTag, true );
            enableAction( DeleteTag, true );
        }
    }
    else {
        enableAction( CreateTag, true );
        enableAction( ModifyTag, false );
        enableAction( DeleteTag, false );
    }
}

void StandardActionManager::slotCreateNetFeed()
{
    NetFeedCreateDialog *dialog = new NetFeedCreateDialog( d->m_parentWidget );

    QList<QPair<QString, QString> > resourceDescriptions;
    const QList<shared_ptr<NetResource> > resources = ResourceManager::self()->netResources();
    Q_FOREACH( const shared_ptr<NetResource>& resource, resources ) {
        resourceDescriptions.append( QPair<QString, QString>( resource->id(), resource->name() ) );
    }

    dialog->setResourceDescriptions( resourceDescriptions );
    if ( dialog->exec() == KDialog::Accepted ) {
        const QString identifier = dialog->resourceIdentifier();
        const QString url = dialog->url();

        Q_FOREACH( const shared_ptr<NetResource>& resource, resources ) {
            if ( resource->id() == identifier ) {
                NetFeedCreateJob* const job = resource->netFeedCreateJob( url );
                connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotNetFeedCreated( KJob* ) ) );
                job->start();
                break;
            }
        }
    }

    delete dialog;
}

void StandardActionManager::slotFetchFeed()
{
    // FIXME: only single selections
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    if ( !selectedIndex.isValid() )
        return;

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<FeedNode> feedNode = boost::dynamic_pointer_cast<FeedNode, TreeNode>( treeNode );
    if ( !feedNode )
        return;

    d->m_feedList->constFeedById( feedNode->feedId() )->fetch();
}

void StandardActionManager::slotAbortFetch()
{
    // FIXME: only single selections
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    if ( !selectedIndex.isValid() )
        return;

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<FeedNode> feedNode = boost::dynamic_pointer_cast<FeedNode, TreeNode>( treeNode );
    if ( !feedNode )
        return;

    d->m_feedList->constFeedById( feedNode->feedId() )->abortFetch();
}

void StandardActionManager::slotFeedProperties()
{
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    if ( !selectedIndex.isValid() )
        return;

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<FeedNode> feedNode = boost::dynamic_pointer_cast<FeedNode, TreeNode>( treeNode );
    if ( !feedNode )
        return;

    shared_ptr<NetFeed> feed = dynamic_pointer_cast<NetFeed, Feed>( d->m_feedList->feedById( feedNode->feedId() ) );

    FeedPropertiesDialog *dialog = new FeedPropertiesDialog();
    dialog->setFeedTitle( feed->title() );
    dialog->setUrl( feed->xmlUrl() );
    dialog->setCustomFetchInterval( true ); // currently we don't use default settings
    dialog->setFetchInterval( feed->fetchInterval() );

    if ( dialog->exec() == KDialog::Accepted ) {
        feed->setTitle( dialog->feedTitle() );
        feed->setXmlUrl( dialog->url() );
        dialog->hasCustomFetchInterval() ? feed->setFetchInterval( dialog->fetchInterval() ) :
                                           feed->setFetchInterval( 0 );

        FeedModifyJob * const job = new FeedModifyJob( feed, this );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedModified( KJob* ) ) );
        job->start();
    }

    delete dialog;
}

void StandardActionManager::slotDeleteFeed()
{
    // FIXME: only single selections
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    if ( !selectedIndex.isValid() )
        return;

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<FeedNode> feedNode = boost::dynamic_pointer_cast<FeedNode, TreeNode>( treeNode );
    if ( !feedNode )
        return;

    shared_ptr<const Feed> feed = d->m_feedList->constFeedById( feedNode->feedId() );
    Q_ASSERT( feed );

    FeedDeleteJob * const job = new FeedDeleteJob( feed , this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedDeleted( KJob* ) ) );
    job->start();
}

void StandardActionManager::slotCreateTag()
{
    TagPropertiesDialog *dialog = new TagPropertiesDialog( d->m_parentWidget );
    if ( dialog->exec() == KDialog::Accepted ) {
        Tag newTag;
        newTag.setLabel( dialog->label() );
        newTag.setDescription( dialog->description() );

        TagCreateJob *job = d->m_tagProvider->tagCreateJob();
        job->setTag( newTag );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTagCreated( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotModifyTag()
{
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<TagNode> tagNode = boost::dynamic_pointer_cast<TagNode, TreeNode>( treeNode );
    if ( !tagNode )
        return;

    Tag tag = tagNode->tag();
    TagPropertiesDialog *dialog = new TagPropertiesDialog( d->m_parentWidget );
    dialog->setLabel( tag.label() );
    dialog->setDescription( tag.description() );
    if ( dialog->exec() == KDialog::Accepted ) {
        tag.setLabel( dialog->label() );
        tag.setDescription( dialog->description() );

        TagModifyJob *job = d->m_tagProvider->tagModifyJob();
        job->setTag( tag );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTagModified( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotDeleteTag()
{
    QModelIndex selectedIndex;
    if ( d->m_feedSelectionModel && d->m_feedSelectionModel->hasSelection() )
        selectedIndex = d->m_feedSelectionModel->selectedRows().first();

    const shared_ptr<TreeNode> treeNode = selectedIndex.data( FeedListModel::TreeNodeRole ).
                                           value<shared_ptr<TreeNode> >();
    const shared_ptr<TagNode> tagNode = boost::dynamic_pointer_cast<TagNode, TreeNode>( treeNode );
    if ( !tagNode )
        return;

    const Tag tag = tagNode->tag();
    TagDeleteJob *job = d->m_tagProvider->tagDeleteJob();
    job->setTag( tag );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTagDeleted( KJob* ) ) );
    job->start();
}

void StandardActionManager::slotManageSubscriptions()
{
    KDialog *dialog = new KDialog( d->m_parentWidget );
    dialog->setCaption( i18n( "Manage RSS subscriptions" ) );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );

#ifdef TEMPORARILY_REMOVED
    FeedList *allFeeds = new FeedList( ResourceManager::self()->identifiers(), dialog );
    SubscriptionsModel *subscriptionsModel = new SubscriptionsModel( d->m_subscriptionLabel, dialog );
    subscriptionsModel->setFeedList( allFeeds );
    QTreeView *subscriptionsView = new QTreeView( dialog );
    subscriptionsView->setModel( subscriptionsModel );
    dialog->setMainWidget( subscriptionsView );
    dialog->setInitialSize( QSize( 400, 460 ) );

    if ( dialog->exec() == KDialog::Accepted ) {
        Q_FOREACH( Feed *feed, subscriptionsModel->unsubscribed() ) {
            kDebug() << "Unsubscribing from:" << feed->id() << ", title:" << feed->title();
            feed->removeSubscriptionLabel( d->m_subscriptionLabel );
            FeedModifyJob *job = new FeedModifyJob( static_cast<const Feed *> ( feed ), this );
            connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedModified( KJob* ) ) );
            job->start();
        }
        Q_FOREACH( Feed *feed, subscriptionsModel->subscribed() ) {
            kDebug() << "Subscribing to:" << feed->id() << ", title:" << feed->title();
            feed->addSubscriptionLabel( d->m_subscriptionLabel );
            FeedModifyJob *job = new FeedModifyJob( static_cast<const Feed *> ( feed ), this );
            connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedModified( KJob* ) ) );
            job->start();
        }
    }

#endif

    delete dialog;
}

void StandardActionManager::slotMarkItemNew()
{
    QModelIndexList selectedIndexes;
    if ( d->m_itemSelectionModel && d->m_itemSelectionModel->hasSelection() )
        selectedIndexes = d->m_itemSelectionModel->selectedRows();
    else
        return;

    Q_FOREACH( const QModelIndex &selectedIndex, selectedIndexes ) {
        Item item = selectedIndex.data( ItemModel::ItemRole ).value<Item>();

        if ( item.status().testFlag( KRss::Item::New ) )
            continue;

        // set 'New' and 'Unread'
        item.setStatus( item.status() | KRss::Item::New | KRss::Item::Unread );
        ItemModifyJob * const job = new ItemModifyJob();
        job->setItem( item );
        job->setIgnorePayload( true );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotMarkItemRead()
{
    QModelIndexList selectedIndexes;
    if ( d->m_itemSelectionModel && d->m_itemSelectionModel->hasSelection() )
        selectedIndexes = d->m_itemSelectionModel->selectedRows();
    else
        return;

    Q_FOREACH( const QModelIndex &selectedIndex, selectedIndexes ) {
        Item item = selectedIndex.data( ItemModel::ItemRole ).value<Item>();

        if ( !item.status().testFlag( KRss::Item::Unread ) )
            continue;

        // clear 'New' and 'Unread'
        item.setStatus( item.status() & ~( KRss::Item::New | KRss::Item::Unread ) );
        ItemModifyJob * const job = new ItemModifyJob();
        job->setItem( item );
        job->setIgnorePayload( true );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotMarkItemUnread()
{
    QModelIndexList selectedIndexes;
    if ( d->m_itemSelectionModel && d->m_itemSelectionModel->hasSelection() )
        selectedIndexes = d->m_itemSelectionModel->selectedRows();
    else
        return;

    Q_FOREACH( const QModelIndex &selectedIndex, selectedIndexes ) {
        Item item = selectedIndex.data( ItemModel::ItemRole ).value<Item>();

        if ( item.status().testFlag( KRss::Item::Unread ) )
            continue;

        // set 'Unread'
        item.setStatus( item.status() | KRss::Item::Unread );
        ItemModifyJob * const job = new ItemModifyJob();
        job->setItem( item );
        job->setIgnorePayload( true );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotMarkItemImportant( bool checked )
{
    QModelIndexList selectedIndexes;
    if ( d->m_itemSelectionModel && d->m_itemSelectionModel->hasSelection() )
        selectedIndexes = d->m_itemSelectionModel->selectedRows();
    else
        return;

    Q_FOREACH( const QModelIndex &selectedIndex, selectedIndexes ) {
        Item item = selectedIndex.data( ItemModel::ItemRole ).value<Item>();

        // set or clear 'Important' according to 'checked'
        ( checked ? item.setStatus( item.status() | KRss::Item::Important ) :
                    item.setStatus( item.status() & ~KRss::Item::Important ) );

        ItemModifyJob * const job = new ItemModifyJob();
        job->setItem( item );
        job->setIgnorePayload( true );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotDeleteItem()
{
    QModelIndexList selectedIndexes;
    if ( d->m_itemSelectionModel && d->m_itemSelectionModel->hasSelection() )
        selectedIndexes = d->m_itemSelectionModel->selectedRows();
    else
        return;

    Q_FOREACH( const QModelIndex &selectedIndex, selectedIndexes ) {
        const Item item = selectedIndex.data( ItemModel::ItemRole ).value<Item>();
        ItemDeleteJob * const job = new ItemDeleteJob();
        job->setItem( item );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemDeleted( KJob* ) ) );
        job->start();
    }
}

void StandardActionManager::slotNetFeedCreated( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not add a new feed: %1", job->errorString() ),
                            i18n( "Feed creation failed" ) );
    }
}


void StandardActionManager::slotFeedModified( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not change feed's properties: %1", job->errorString() ),
                            i18n( "Feed properties modification failed" ) );
    }
}

void StandardActionManager::slotFeedDeleted( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not delete the feed: %1", job->errorString() ),
                            i18n( "Feed deletion failed" ) );
    }
}

void StandardActionManager::slotItemModified( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not change item's properties: %1", job->errorString() ),
                            i18n( "Item properties modification failed" ) );
    }
}

void StandardActionManager::slotItemDeleted( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not delete the item: %1", job->errorString() ),
                            i18n( "Item deletion failed" ) );
    }
}

void StandardActionManager::slotTagCreated( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not create tag: %1", job->errorString() ),
                            i18n( "Tag creation failed" ) );
    }
}

void StandardActionManager::slotTagModified( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not modify tag: %1", job->errorString() ),
                            i18n( "Tag modification failed" ) );
    }
}

void StandardActionManager::slotTagDeleted( KJob* job )
{
    if ( job->error() ) {
        KMessageBox::error( d->m_parentWidget, i18n("Could not delete tag: %1", job->errorString() ),
                            i18n( "Tag deletion failed" ) );
    }
}

#include "standardactionmanager.moc"
