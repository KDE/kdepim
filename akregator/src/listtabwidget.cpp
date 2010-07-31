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

#include "listtabwidget.h"
#include "feedlistview.h"
#include "folder.h"
#include "treenode.h"

#include <kmultitabbar.h>

#include <tqiconset.h>
#include <tqlayout.h>
#include <tqmap.h>
#include <tqptrlist.h>
#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqwidgetstack.h>

#include <kdebug.h>

namespace Akregator {

class ListTabWidget::ListTabWidgetPrivate
{

public:
    int idCounter;
    KMultiTabBar* tabBar;
    TQWidgetStack* stack;
    NodeListView* current;
    int currentID;
    TQValueList<NodeListView*> views;
    TQMap<int, NodeListView*> idToView;
    TQHBoxLayout* layout;
    ViewMode viewMode;
    TQMap<TQWidget*, TQString> captions;
};


void ListTabWidget::slotItemUp()
{
    if (d->current)
        d->current->slotItemUp();
}

void ListTabWidget::slotItemDown()
{
    if (d->current)
        d->current->slotItemDown();
}

void ListTabWidget::slotItemBegin()
{
    if (d->current)
        d->current->slotItemBegin();
}

void ListTabWidget::slotItemEnd()
{
    if (d->current)
        d->current->slotItemEnd();
}

void ListTabWidget::slotItemLeft()
{
    if (d->current)
        d->current->slotItemLeft();
}

void ListTabWidget::slotItemRight()
{
    if (d->current)
        d->current->slotItemRight();
}

void ListTabWidget::slotPrevFeed()
{
    if (d->current)
        d->current->slotPrevFeed();
}

void ListTabWidget::slotNextFeed()
{
    if (d->current)
        d->current->slotNextFeed();
}

void ListTabWidget::slotPrevUnreadFeed()
{
    if (d->current)
        d->current->slotPrevUnreadFeed();
}

void ListTabWidget::slotNextUnreadFeed()
{
    if (d->current)
        d->current->slotNextUnreadFeed();
}

void ListTabWidget::slotRootNodeChanged(NodeListView* view, TreeNode* node)
{
/*
    int unread = node->unread();
    if (unread > 0)
    {
        //uncomment this to append unread count
        //d->tabWidget->changeTab(view,  TQString("<qt>%1 (%2)").arg(d->captions[view]).arg(node->unread()));
        d->tabWidget->changeTab(view, d->captions[view]);
    }
    else
    {
        d->tabWidget->changeTab(view, d->captions[view]);
    }
*/
}

void ListTabWidget::slotTabClicked(int id)
{
    NodeListView* view = d->idToView[id];
    if (view)
    {
        d->stack->raiseWidget(view);
        d->current = view;

        if (d->currentID >= 0)
            d->tabBar->setTab(d->currentID, false);
        d->currentID = id;
        d->tabBar->setTab(d->currentID, true);

        emit signalNodeSelected(d->current->selectedNode());
    }
}

ListTabWidget::ListTabWidget(TQWidget* parent, const char* name) : TQWidget(parent, name), d(new ListTabWidgetPrivate)
{
    d->idCounter = 0;
    d->current = 0;
    d->currentID = -1;
    d->viewMode = verticalTabs;
    d->layout = new TQHBoxLayout(this);
    //d->layout = new TQGridLayout(this, 1, 2);
    d->tabBar = new KMultiTabBar(KMultiTabBar::Vertical, this); 
    d->tabBar->setStyle(KMultiTabBar::KDEV3ICON);
    //d->tabBar->setStyle(KMultiTabBar::KDEV3);
    d->tabBar->showActiveTabTexts(true);
    d->tabBar->setPosition(KMultiTabBar::Left);
    d->layout->addWidget(d->tabBar/*, 0, 0*/);

    d->stack = new TQWidgetStack(this);
    d->layout->addWidget(d->stack/*, 0, 1*/);
    
//    connect(d->tabBar, TQT_SIGNAL(currentChanged(TQWidget*)), this, TQT_SLOT(slotCurrentChanged(TQWidget*)));
}

ListTabWidget::~ListTabWidget()
{
    delete d;
    d = 0;
}


void ListTabWidget::setViewMode(ViewMode mode)
{
    if (mode == d->viewMode)
        return;

    d->viewMode = mode;

    // if mode is "single", we hide the tab bar
    d->tabBar->setHidden(mode == single);
}

ListTabWidget::ViewMode ListTabWidget::viewMode() const
{
    return d->viewMode;
}

void ListTabWidget::addView(NodeListView* view, const TQString& caption, const TQPixmap& icon)
{
    d->captions[view] = caption;    

    view->reparent(d->stack, TQPoint(0,0));
    d->stack->addWidget(view);
   
    int tabId = d->idCounter++;
    d->tabBar->appendTab(icon, tabId, caption);
    d->idToView[tabId] = view;
    connect(d->tabBar->tab(tabId), TQT_SIGNAL(clicked(int)), this, TQT_SLOT(slotTabClicked(int)));

    
    connect(view, TQT_SIGNAL(signalNodeSelected(TreeNode*)), this, TQT_SIGNAL(signalNodeSelected(TreeNode*)));
    connect(view, TQT_SIGNAL(signalRootNodeChanged(NodeListView*, TreeNode*)), this, TQT_SLOT(slotRootNodeChanged(NodeListView*, TreeNode*)));


    if (tabId == 0) // first widget
    {
        d->current = view;
        d->currentID = tabId;
        d->tabBar->setTab(d->currentID, true);
        d->stack->raiseWidget(view);
    }
}

NodeListView* ListTabWidget::activeView() const
{
    return d->current;
}

}

#include "listtabwidget.h"

#include "listtabwidget.moc"
