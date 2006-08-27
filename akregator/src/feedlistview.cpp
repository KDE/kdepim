/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "dragobjects.h"
#include "folder.h"
#include "folderitem.h"
#include "tagfolder.h"
#include "tagfolderitem.h"
#include "feedlistview.h"
#include "feed.h"
#include "feeditem.h"
#include "feedlist.h"
#include "tag.h"
#include "tagnode.h"
#include "tagnodeitem.h"
#include "tagnodelist.h"
#include "treenode.h"
#include "treenodeitem.h"
#include "treenodevisitor.h"

#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmultipledrag.h>
#include <kstringhandler.h>
#include <kurldrag.h>

#include <qfont.h>
#include <qheader.h>
#include <qpainter.h>
#include <qptrdict.h>
#include <qtimer.h>
#include <qwhatsthis.h>

namespace Akregator {

class NodeListView::NodeListViewPrivate
{
    public:
/** used for finding the item belonging to a node */
    QPtrDict<TreeNodeItem> itemDict;
    NodeList* nodeList;
    bool showTagFolders;

    // Drag and Drop variables
    QListViewItem *parent;
    QListViewItem *afterme;
    QTimer autoopentimer;
    ConnectNodeVisitor* connectNodeVisitor;
    DisconnectNodeVisitor* disconnectNodeVisitor;
    CreateItemVisitor* createItemVisitor;
    DeleteItemVisitor* deleteItemVisitor;
};

class NodeListView::ConnectNodeVisitor : public TreeNodeVisitor
{
    public:
        ConnectNodeVisitor(NodeListView* view) : m_view(view) {}

        virtual bool visitTreeNode(TreeNode* node)
        {
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            connect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            visitTreeNode(node);
            connect(node, SIGNAL(signalChildAdded(TreeNode*)), m_view, SLOT(slotNodeAdded(TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_view, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            visitTreeNode(node);
            
            connect(node, SIGNAL(fetchStarted(Feed*)), m_view, SLOT(slotFeedFetchStarted(Feed*)));
            connect(node, SIGNAL(fetchAborted(Feed*)), m_view, SLOT(slotFeedFetchAborted(Feed*)));
            connect(node, SIGNAL(fetchError(Feed*)), m_view, SLOT(slotFeedFetchError(Feed*)));
            connect(node, SIGNAL(fetched(Feed*)), m_view, SLOT(slotFeedFetchCompleted(Feed*)));
            return true;
        }
    private:

        NodeListView* m_view;
    
};

class NodeListView::DisconnectNodeVisitor : public TreeNodeVisitor
{
    public:
        DisconnectNodeVisitor(NodeListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
            disconnect(node, SIGNAL(signalChildAdded(TreeNode*)), m_view, SLOT(slotNodeAdded(TreeNode*) ));
            disconnect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_view, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            
            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {

            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_view, SLOT(slotNodeDestroyed(TreeNode*) ));
            disconnect(node, SIGNAL(signalChanged(TreeNode*)), m_view, SLOT(slotNodeChanged(TreeNode*) ));
            disconnect(node, SIGNAL(fetchStarted(Feed*)), m_view, SLOT(slotFeedFetchStarted(Feed*)));
            disconnect(node, SIGNAL(fetchAborted(Feed*)), m_view, SLOT(slotFeedFetchAborted(Feed*)));
            disconnect(node, SIGNAL(fetchError(Feed*)), m_view, SLOT(slotFeedFetchError(Feed*)));
            disconnect(node, SIGNAL(fetched(Feed*)), m_view, SLOT(slotFeedFetchCompleted(Feed*)));
            return true;
        }
    private:

        NodeListView* m_view;
};

class NodeListView::DeleteItemVisitor : public TreeNodeVisitor
{
    public:
        
        DeleteItemVisitor(NodeListView* view) : m_view(view) {}
        
        virtual bool visitTreeNode(TreeNode* node)
        {
            TreeNodeItem* item = m_view->d->itemDict.take(node);
    
            if (!item)
                return true;
    
            if ( m_selectNeighbour && item->isSelected() )
            {
                if (item->itemBelow())
                    m_view->setSelected(item->itemBelow(), true);
                else if (item->itemAbove())
                    m_view->setSelected(item->itemAbove(), true);
                else
                    m_view->setSelected(item, false);
            }
            
            m_view->disconnectFromNode(node);
            delete item;
            return true;
        
        }
        
        virtual bool visitFolder(Folder* node)
        {
            // delete child items recursively before deleting parent
            QValueList<TreeNode*> children = node->children();
            for (QValueList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);
            
            visitTreeNode(node);
            
            return true;
        }
        
        void deleteItem(TreeNode* node, bool selectNeighbour)
        {
            m_selectNeighbour = selectNeighbour;
            visit(node);
        }
        
    private:
        NodeListView* m_view;
        bool m_selectNeighbour;
};

class NodeListView::CreateItemVisitor : public TreeNodeVisitor
{
    public:
        CreateItemVisitor(NodeListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* node)
        {
            if (m_view->findNodeItem(node))
                return true;
            
            TagNodeItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new TagNodeItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagNodeItem( parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new TagNodeItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagNodeItem(m_view, node);
            }                
            item->nodeChanged();     
            m_view->d->itemDict.insert(node, item);
            m_view->connectToNode(node);
            if (parentItem)
                parentItem->sortChildItems(0, true);
            return true;
        }

        virtual bool visitTagFolder(TagFolder* node)
        {
            if (m_view->findNodeItem(node))
                return true;
         
            TagFolderItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new TagFolderItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagFolderItem(parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new TagFolderItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new TagFolderItem(m_view, node);

            }
            m_view->d->itemDict.insert(node, item);
            QValueList<TreeNode*> children = node->children();

            // add children recursively
            for (QValueList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);

            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFolder(Folder* node)
        {
            if (m_view->findNodeItem(node))
                return true;
                     
            FolderItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            if (parentItem)
            {
                if (prev)
                {
                    item = new FolderItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FolderItem(parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new FolderItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FolderItem(m_view, node);
            }
            m_view->d->itemDict.insert(node, item);
            
            // add children recursively
            QValueList<TreeNode*> children = node->children();
            for (QValueList<TreeNode*>::ConstIterator it =  children.begin(); it != children.end(); ++it )
                visit(*it);

            m_view->connectToNode(node);
            return true;
        }
        
        virtual bool visitFeed(Feed* node)
        {
            if (m_view->findNodeItem(node))
                return true;
         
            FeedItem* item = 0;
            TreeNode* prev = node->prevSibling();
            FolderItem* parentItem = static_cast<FolderItem*>(m_view->findNodeItem(node->parent()));
            
            if (parentItem)
            {
                if (prev)
                {
                    item = new FeedItem( parentItem, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FeedItem( parentItem, node);
            }
            else
            {
                if (prev)
                {
                    item = new FeedItem(m_view, m_view->findNodeItem(prev), node);
                }
                else
                    item = new FeedItem(m_view, node);
            }

            item->nodeChanged();     
            m_view->d->itemDict.insert(node, item);
            m_view->connectToNode(node);
            return true;
        }
        
    private:
        NodeListView* m_view;
};

NodeListView::NodeListView( QWidget *parent, const char *name)
        : KListView(parent, name), d(new NodeListViewPrivate)
{
    d->showTagFolders = true;
    d->connectNodeVisitor = new ConnectNodeVisitor(this),
    d->disconnectNodeVisitor = new DisconnectNodeVisitor(this);
    d->createItemVisitor = new CreateItemVisitor(this);
    d->deleteItemVisitor = new DeleteItemVisitor(this);

    setMinimumSize(150, 150);
    addColumn(i18n("Feeds"));
    setRootIsDecorated(false);
    setItemsRenameable(false); // NOTE: setting this this to true collides with setRenameEnabled() in items and breaks in-place renaming in strange ways. Do not enable!
    setItemMargin(2);

    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);
    //setDropHighlighter(false);

    setDragEnabled(true);
    setAcceptDrops(true);
    setItemsMovable(true);
    
    connect( this, SIGNAL(dropped(QDropEvent*, QListViewItem*)), this, SLOT(slotDropped(QDropEvent*, QListViewItem*)) );
    connect( this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*, int, const QString&)), this, SLOT(slotItemRenamed(QListViewItem*, int, const QString&)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)), this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect( &(d->autoopentimer), SIGNAL( timeout() ), this, SLOT( openFolder() ) );

    clear();
    
    QWhatsThis::add(this, i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
    setUpdatesEnabled(true);
}

NodeListView::~NodeListView()
{
    delete d->connectNodeVisitor;
    delete d->disconnectNodeVisitor;
    delete d->createItemVisitor;
    delete d->deleteItemVisitor;
    delete d;
    d = 0;
}

void NodeListView::setNodeList(NodeList* nodeList)
{
    if (nodeList == d->nodeList)
         return;

    clear();

    disconnectFromNodeList(d->nodeList);
    
    if (!nodeList)
        return;

    d->nodeList = nodeList;
    connectToNodeList(nodeList);
  
    
    Folder* rootNode = nodeList->rootNode();
    if (!rootNode)
        return;

    slotNodeAdded(rootNode);
    slotRootNodeChanged(rootNode);
}

Folder* NodeListView::rootNode()
{
    return d->nodeList ? d->nodeList->rootNode() : 0;
}

TreeNode* NodeListView::selectedNode()
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*> (selectedItem());
    
    return ( item ? item->node() : 0) ;
}

void NodeListView::setSelectedNode(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if ( node && item )
        setSelected(item, true);
}

TreeNode* NodeListView::findNodeByTitle(const QString& title)
{
    TreeNodeItem* item = dynamic_cast<TreeNodeItem*>(findItemByTitle(title, 0));
    if (!item)
        return 0;
    else 
        return item->node();
}

TreeNodeItem* NodeListView::findNodeItem(TreeNode* node)
{
    return d->itemDict.find(node);
}

TreeNodeItem* NodeListView::findItemByTitle(const QString& text, int column, ComparisonFlags compare) const
{ 
    return dynamic_cast<TreeNodeItem*> (KListView::findItem(text, column, compare)); 
}

void NodeListView::ensureNodeVisible(TreeNode* node)
{
    ensureItemVisible(findNodeItem(node));
}

void NodeListView::startNodeRenaming(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {   
        item->startRename(0);
    }
}

void NodeListView::clear()
{
    QPtrDictIterator<TreeNodeItem> it(d->itemDict);
    for( ; it.current(); ++it )
        disconnectFromNode( it.current()->node() );
    d->itemDict.clear();
    d->nodeList = 0;
    
    KListView::clear();
}

void NodeListView::drawContentsOffset( QPainter * p, int ox, int oy,
                                       int cx, int cy, int cw, int ch )
{
    bool oldUpdatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    setUpdatesEnabled(oldUpdatesEnabled);
}

void NodeListView::slotDropped( QDropEvent *e, QListViewItem*
/*after*/)
{
	d->autoopentimer.stop();

    if (e->source() != viewport())
    {
        openFolder();

        if (KURLDrag::canDecode(e))
        {
            FolderItem* parent = dynamic_cast<FolderItem*> (d->parent);
            TreeNodeItem* afterMe = 0;
            
            if(d->afterme)
                afterMe = dynamic_cast<TreeNodeItem*> (d->afterme);
            
            KURL::List urls;
            KURLDrag::decode( e, urls );
            e->accept();
            emit signalDropped( urls, afterMe ? afterMe->node() : 0, parent ? parent->node() : 0);
        }
    }
    else
    {
    }
}

void NodeListView::movableDropEvent(QListViewItem* /*parent*/, QListViewItem* /*afterme*/)
{
	d->autoopentimer.stop();
    if (d->parent)
    {    
        openFolder();

        Folder* parentNode = (dynamic_cast<FolderItem*> (d->parent))->node();
        TreeNode* afterMeNode = 0; 
        TreeNode* current = selectedNode();

        if (d->afterme)
            afterMeNode = (dynamic_cast<TreeNodeItem*> (d->afterme))->node();

        current->parent()->removeChild(current);
        parentNode->insertChild(current, afterMeNode);
        KListView::movableDropEvent(d->parent, d->afterme);
    }    
}

void NodeListView::setShowTagFolders(bool enabled)
{
    d->showTagFolders = enabled;
}

void NodeListView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    QPoint vp = contentsToViewport(event->pos());
    QListViewItem *i = itemAt(vp);

    QListViewItem *qiparent;
    QListViewItem *qiafterme;
    findDrop( event->pos(), qiparent, qiafterme );

    if (event->source() == viewport()) {
        // disable any drops where the result would be top level nodes 
        if (i && !i->parent())
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }

        // prevent dragging nodes from All Feeds to My Tags or vice versa
        QListViewItem* root1 = i;
        while (root1 && root1->parent())
            root1 = root1->parent();

        QListViewItem* root2 = selectedItem();
        while (root2 && root2->parent())
            root2 = root2->parent();

        if (root1 != root2)
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }

        // don't drop node into own subtree
        QListViewItem* p = qiparent;
        while (p)
            if (p == selectedItem())
            {
                event->ignore();
                d->autoopentimer.stop();
                return;
            }
            else
            {
                p = p->parent();
            }

        // disable drags onto the item itself
        if (selectedItem() == i)
        {
            event->ignore();
            d->autoopentimer.stop();
            return;
        }
    }

    // what the hell was this good for? -fo
    //    if (!i || event->pos().x() > header()->cellPos(header()->mapToIndex(0)) +
    //            treeStepSize() * (i->depth() + 1) + itemMargin() ||
    //            event->pos().x() < header()->cellPos(header()->mapToIndex(0)))
    //   {} else
 
    // do we want to move inside the old parent or do we want to move to a new parent
    if (i && (itemAt(vp - QPoint(0,5)) == i && itemAt(vp + QPoint(0,5)) == i))
    {
        setDropVisualizer(false);
        setDropHighlighter(true);
        cleanDropVisualizer();

        TreeNode *iNode = (dynamic_cast<TreeNodeItem*> (i))->node();
        if (iNode->isGroup())
        {
            if (i != d->parent)
                d->autoopentimer.start(750);

            d->parent = i;
            d->afterme = 0;
        }
        else
        {
            event->ignore();
            d->autoopentimer.stop();
            d->afterme = i;
            return;
        }
    }
    else
    {
        setDropVisualizer(true);
        setDropHighlighter(false);
        cleanItemHighlighter();
        d->parent = qiparent;
        d->afterme = qiafterme;
        d->autoopentimer.stop();
    }

    // the rest is handled by KListView.
    KListView::contentsDragMoveEvent(event);
}

bool NodeListView::acceptDrag(QDropEvent *e) const
{
    if (!acceptDrops() || !itemsMovable())
        return false;

    if (e->source() != viewport())
    {
        return KURLDrag::canDecode(e);
    }
    else
    {
        // disable dragging of top-level nodes (All Feeds, My Tags)
        if (selectedItem() && !selectedItem()->parent())
            return false;
        else
            return true;
    }

    return true;
}

void NodeListView::slotItemUp()
{
    if (selectedItem() && selectedItem()->itemAbove())
    {
        setSelected( selectedItem()->itemAbove(), true );
        ensureItemVisible(selectedItem());
    }   
}

void NodeListView::slotItemDown()
{
    if (selectedItem() && selectedItem()->itemBelow())
    {    
        setSelected( selectedItem()->itemBelow(), true );
        ensureItemVisible(selectedItem());
    }
}

void NodeListView::slotItemBegin()
{
    setSelected( firstChild(), true );
    ensureItemVisible(firstChild());
}

void NodeListView::slotItemEnd()
{
    QListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setSelected( elt, true );
    ensureItemVisible(elt);
}

void NodeListView::slotItemLeft()
{
    QListViewItem* sel = selectedItem();
    
    if (!sel || sel == findNodeItem(rootNode()))
        return;
    
    if (sel->isOpen())
        sel->setOpen(false);
    else
    {
        if (sel->parent())
            setSelected( sel->parent(), true );
    }
        
    ensureItemVisible( selectedItem() );    
}

void NodeListView::slotItemRight()
{
    QListViewItem* sel = selectedItem();
    if (!sel)
    {
        setSelected( firstChild(), true );
        sel = firstChild();
    }
    if (sel->isExpandable() && !sel->isOpen())
        sel->setOpen(true);
    else
    {
        if (sel->firstChild())
            setSelected( sel->firstChild(), true );
    }
    ensureItemVisible( selectedItem() );
}

void NodeListView::slotPrevFeed()
{
    for (QListViewItemIterator it( selectedItem()); it.current(); --it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*>(*it);
        if (tni && !tni->isSelected() && !tni->node()->isGroup() )
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }     
    }
}
    
void NodeListView::slotNextFeed()
{
    for (QListViewItemIterator it( selectedItem()); it.current(); ++it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*>(*it);
        if ( tni && !tni->isSelected() && !tni->node()->isGroup() )
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }     
    }
}

void NodeListView::slotPrevUnreadFeed()
{
    if (!firstChild() || !firstChild()->firstChild())
        return;
    if ( !selectedItem() )
        slotNextUnreadFeed(); 

    QListViewItemIterator it( selectedItem() );
    
    for ( ; it.current(); --it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }
    }
    // reached when there is no unread feed above the selected one
    // => cycle: go to end of list...
    if (rootNode()->unread() > 0)
    {

        it = QListViewItemIterator(lastItem());
    
        for ( ; it.current(); --it)
        {

            TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());

            if (!tni)
                break;

            if (!tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
            {
                setSelected(tni, true);
                ensureItemVisible(tni);
                return;
            }
        }
    }
}

void NodeListView::slotNextUnreadFeed()
{
    QListViewItemIterator it;
    
    if ( !selectedItem() )
    {
        // if all feeds doesnt exists or is empty, return
        if (!firstChild() || !firstChild()->firstChild())
            return;    
        else 
            it = QListViewItemIterator( firstChild()->firstChild());
    }
    else
        it = QListViewItemIterator( selectedItem() );
    
    for ( ; it.current(); ++it )
    {
        TreeNodeItem* tni = dynamic_cast<TreeNodeItem*> (it.current());
        if (!tni)
            break;
        if ( !tni->isSelected() && !tni->node()->isGroup() && tni->node()->unread() > 0)
        {
            setSelected(tni, true);
            ensureItemVisible(tni);
            return;
        }
    }
    // if reached, we are at the end of the list++
    if (rootNode()->unread() > 0)
    {
        clearSelection();
        slotNextUnreadFeed();
    }
}

void NodeListView::slotSelectionChanged(QListViewItem* item)
{
    TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    
    if (ni)
    {
        emit signalNodeSelected(ni->node());
    }
}

void NodeListView::slotItemRenamed(QListViewItem* item, int col, const QString& text)
{
    TreeNodeItem* ni = dynamic_cast<TreeNodeItem*> (item);
    if ( !ni || !ni->node() )
        return;
    if (col == 0)
    {
        if (text != ni->node()->title())
        {
            ni->node()->setTitle(text);
        }
    }
}
void NodeListView::slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p)
{    
    TreeNodeItem* ti = dynamic_cast<TreeNodeItem*>(item);
    emit signalContextMenu(list, ti ? ti->node() : 0, p);
    if (ti)
        ti->showContextMenu(p);
}

void NodeListView::slotFeedFetchStarted(Feed* feed)
{
    // Disable icon to show it is fetching.
    if (!feed->favicon().isNull())
    {
        TreeNodeItem* item = findNodeItem(feed);
        if (item)
        {
            KIconEffect iconEffect;
            QPixmap tempIcon = iconEffect.apply(feed->favicon(), KIcon::Small, KIcon::DisabledState);
            item->setPixmap(0, tempIcon);
        }
    }

}

void NodeListView::slotFeedFetchAborted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void NodeListView::slotFeedFetchError(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}

void NodeListView::slotFeedFetchCompleted(Feed* feed)
{
    TreeNodeItem* item = findNodeItem(feed);
    if (item)
        item->nodeChanged();
}
      
void NodeListView::slotNodeAdded(TreeNode* node)
{
    if (node)
        d->createItemVisitor->visit(node);
}

void NodeListView::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if (node)
        d->deleteItemVisitor->deleteItem(node, false);
}

void NodeListView::connectToNode(TreeNode* node)
{
    if (node)
        d->connectNodeVisitor->visit(node);
}

void NodeListView::connectToNodeList(NodeList* list)
{
    if (!list)
        return;
    
    connect(list, SIGNAL(signalDestroyed(NodeList*)), this, SLOT(slotNodeListDestroyed(NodeList*)) );
    connect(list->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotRootNodeChanged(TreeNode*)));
}

void NodeListView::disconnectFromNodeList(NodeList* list)
{
    if (!list)
        return;
    
    disconnect(list, SIGNAL(signalDestroyed(NodeList*)), this, SLOT(slotNodeListDestroyed(NodeList*)) );
    disconnect(list->rootNode(), SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotRootNodeChanged(TreeNode*)));
}

void NodeListView::disconnectFromNode(TreeNode* node)
{
    if (node)
        d->disconnectNodeVisitor->visit(node);
}

void NodeListView::slotNodeListDestroyed(NodeList* list)
{
    if (list != d->nodeList)
        return;

    setNodeList(0);
}

void NodeListView::slotNodeDestroyed(TreeNode* node)
{
    if (node)
        d->deleteItemVisitor->deleteItem(node, true);
}

void NodeListView::slotRootNodeChanged(TreeNode* rootNode)
{
    emit signalRootNodeChanged(this, rootNode);
}

void NodeListView::slotNodeChanged(TreeNode* node)
{
    TreeNodeItem* item = findNodeItem(node);
    if (item)
    {    
        item->nodeChanged();
        triggerUpdate();
    }    
}

QDragObject *NodeListView::dragObject()
{
    KMultipleDrag *md = new KMultipleDrag(viewport());
    QDragObject *obj = KListView::dragObject();
    if (obj) {
        md->addDragObject(obj);
    }
    TreeNodeItem *i = dynamic_cast<TreeNodeItem*>(currentItem());
    if (i) {
        md->setPixmap(*(i->pixmap(0)));
        FeedItem *fi = dynamic_cast<FeedItem*>(i);
        if (fi) {
            md->addDragObject(new KURLDrag(KURL(fi->node()->xmlUrl()), 0L));
        }
    }
    return md;
}

void NodeListView::openFolder() {
    d->autoopentimer.stop();
    if (d->parent && !d->parent->isOpen())
    {
        d->parent->setOpen(true);
    }
}

} // namespace Akregator

#include "feedlistview.moc"
