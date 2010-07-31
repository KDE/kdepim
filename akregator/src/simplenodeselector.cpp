/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "feedlist.h"
#include "folder.h"
#include "simplenodeselector.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <klistview.h>
#include <klocale.h>

#include <tqlayout.h>
#include <tqmap.h>
#include <tqwidget.h>

namespace Akregator
{

class SelectNodeDialog::SelectNodeDialogPrivate
{
    public:
    SimpleNodeSelector* widget;
};

SelectNodeDialog::SelectNodeDialog(FeedList* feedList, TQWidget* parent, char* name) : 
 KDialogBase(parent, name, true, i18n("Select Feed or Folder"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), d(new SelectNodeDialogPrivate)
{
    d->widget = new SimpleNodeSelector(feedList, this);

    connect(d->widget, TQT_SIGNAL(signalNodeSelected(TreeNode*)), this, TQT_SLOT(slotNodeSelected(TreeNode*)));

    setMainWidget(d->widget);
    enableButtonOK(false);
}

SelectNodeDialog::~SelectNodeDialog()
{
    delete d;
    d = 0;
}

TreeNode* SelectNodeDialog::selectedNode() const
{
    return d->widget->selectedNode();
}

void SelectNodeDialog::slotSelectNode(TreeNode* node)
{
    d->widget->slotSelectNode(node);
}

void SelectNodeDialog::slotNodeSelected(TreeNode* node)
{
    enableButtonOK(node != 0);
}


class SimpleNodeSelector::SimpleNodeSelectorPrivate
{
    public:
    KListView* view;
    FeedList* list;
    NodeVisitor* visitor;
    TQMap<TreeNode*,TQListViewItem*> nodeToItem;
    TQMap<TQListViewItem*, TreeNode*> itemToNode;
};

class SimpleNodeSelector::NodeVisitor : public TreeNodeVisitor
{
    public:

    NodeVisitor(SimpleNodeSelector* view) : TreeNodeVisitor(), m_view(view) {}

    void createItems(TreeNode* node)
    {
        node->accept(this);
    }

    virtual bool visitFolder(Folder* node)
    {
        visitTreeNode(node);
        TQValueList<TreeNode*> children = node->children();
        m_view->d->nodeToItem[node]->setExpandable(true);
        for (TQValueList<TreeNode*>::ConstIterator it = children.begin(); it != children.end(); ++it)
             createItems(*it);
        return true;
    }

    virtual bool visitTreeNode(TreeNode* node)
    {
        TQListViewItem* pi = node->parent() ? m_view->d->nodeToItem[node->parent()] : 0;
         
        KListViewItem* item = 0;
        if (pi != 0)
             item = new KListViewItem(pi, node->title());
        else
             item = new KListViewItem(m_view->d->view, node->title());
        item->setExpandable(false);
        m_view->d->nodeToItem.insert(node, item);
        m_view->d->itemToNode.insert(item, node);
        connect(node, TQT_SIGNAL(signalDestroyed(TreeNode*)), m_view, TQT_SLOT(slotNodeDestroyed(TreeNode*)));
        return true;
    }

    private:

    SimpleNodeSelector* m_view;
};


SimpleNodeSelector::SimpleNodeSelector(FeedList* feedList, TQWidget* parent, const char* name) : TQWidget(parent, name), d(new SimpleNodeSelectorPrivate)
{
    d->list = feedList;
    connect(feedList, TQT_SIGNAL(signalDestroyed(FeedList*)), this, TQT_SLOT(slotFeedListDestroyed(FeedList*)));

    d->view = new KListView(this);
    d->view->setRootIsDecorated(true);
    d->view->addColumn(i18n("Feeds"));
    
    connect(d->view, TQT_SIGNAL(selectionChanged(TQListViewItem*)), this, TQT_SLOT(slotItemSelected(TQListViewItem*)));

    TQGridLayout* layout = new TQGridLayout(this, 1, 1);
    layout->addWidget(d->view, 0, 0);

    d->visitor = new NodeVisitor(this);

    d->visitor->createItems(d->list->rootNode());
    d->nodeToItem[d->list->rootNode()]->setOpen(true);
    d->view->ensureItemVisible(d->nodeToItem[d->list->rootNode()]);
}

SimpleNodeSelector::~SimpleNodeSelector()
{
    delete d->visitor;
    delete d;
    d = 0;
}

TreeNode* SimpleNodeSelector::selectedNode() const
{
    return d->itemToNode[d->view->selectedItem()];
}

void SimpleNodeSelector::slotSelectNode(TreeNode* node)
{
    TQListViewItem* item = d->nodeToItem[node];
    if (item != 0)
        d->view->setSelected(item, true);
}

void SimpleNodeSelector::slotFeedListDestroyed(FeedList* /*list*/)
{
    d->nodeToItem.clear();
    d->itemToNode.clear();
    d->view->clear();
}

void SimpleNodeSelector::slotItemSelected(TQListViewItem* item)
{
    emit signalNodeSelected(d->itemToNode[item]);
}

void SimpleNodeSelector::slotNodeDestroyed(TreeNode* node)
{
    if (d->nodeToItem.contains(node))
    {
        TQListViewItem* item = d->nodeToItem[node];
        d->nodeToItem.remove(node);
        d->itemToNode.remove(item);
        delete item;
    }
}

} // namespace Akregator

#include "simplenodeselector.moc"
	
