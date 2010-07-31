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
#include "tag.h"
#include "tagnode.h"
#include "tagnodelist.h"
#include "tagset.h"
#include "folder.h"
#include "tagfolder.h"

#include <tqdom.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqvaluelist.h>

#include <kapplication.h>
#include <klocale.h>

namespace Akregator {

class TagNodeList::TagNodeListPrivate
{
    public:
    FeedList* feedList;
    TagSet* tagSet;
    TQMap<TQString, TagNode*> tagIdToNodeMap;
};

FeedList* TagNodeList::feedList() const
{
    return d->feedList;
}

TagNodeList::TagNodeList(FeedList* feedList, TagSet* tagSet) :  NodeList(), d(new TagNodeListPrivate)
{
    d->feedList = feedList;
    d->tagSet = tagSet;
 
    connect(d->tagSet, TQT_SIGNAL(signalTagAdded(const Tag&)), this, TQT_SLOT(slotTagAdded(const Tag&)));
    connect(d->tagSet, TQT_SIGNAL(signalTagRemoved(const Tag&)), this, TQT_SLOT(slotTagRemoved(const Tag&)));
    connect(d->tagSet, TQT_SIGNAL(signalTagUpdated(const Tag&)), this, TQT_SLOT(slotTagUpdated(const Tag&)));

    setRootNode(new TagFolder(i18n("My Tags")));

    TQValueList<Tag> list = tagSet->toMap().values();
    for (TQValueList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
       insert(new TagNode(*it, d->feedList->rootNode()));
    }
}

TagNodeList::~TagNodeList()
{
    emit signalDestroyed(this);
    delete d;
    d = 0;
}

TagFolder* TagNodeList::rootNode() const
{
    return static_cast<TagFolder*>(NodeList::rootNode());
}

TagNode* TagNodeList::findByTagID(const TQString& tagID)
{
    return d->tagIdToNodeMap[tagID];
}

bool TagNodeList::insert(TagNode* tagNode)
{
    tagNode->setId(KApplication::random());
    TQString id = tagNode->tag().id();
    if (!containsTagId(id))
    {
        rootNode()->appendChild(tagNode); // TODO: maintain sorting
        d->tagIdToNodeMap[id] = tagNode;
        emit signalTagNodeAdded(tagNode);
        return true;
    }
    return false;
}

bool TagNodeList::remove(TagNode* tagNode)
{
    TQString id = tagNode->tag().id();
    if (containsTagId(id))
    {
        rootNode()->removeChild(tagNode);
        d->tagIdToNodeMap.remove(id);
        emit signalTagNodeRemoved(tagNode);
        return true;
    }
    return false;
}

void TagNodeList::slotNodeDestroyed(TreeNode* node)
{
    TagNode* tagNode = dynamic_cast<TagNode*>(node);
    TQString id = tagNode ? tagNode->tag().id() : TQString::null;
    
    if (tagNode != 0 && containsTagId(id))
    {
        rootNode()->removeChild(tagNode);
        d->tagIdToNodeMap.remove(id);
        emit signalTagNodeRemoved(tagNode);
    }
}

void TagNodeList::slotNodeAdded(TreeNode* node)
{
    NodeList::slotNodeAdded(node);

    TagNode* tagNode = dynamic_cast<TagNode*>(node);
    TQString id = tagNode ? tagNode->tag().id() : TQString::null;
    
    if (tagNode != 0L && !containsTagId(id))
    {
       d->tagIdToNodeMap[id] = tagNode;
       emit signalTagNodeAdded(tagNode);
    }
}

void TagNodeList::slotNodeRemoved(Folder* parent, TreeNode* node)
{
    NodeList::slotNodeRemoved(parent, node);

    TagNode* tagNode = dynamic_cast<TagNode*>(node);
    TQString id = tagNode ? tagNode->tag().id() : TQString::null;
    
    if (parent == rootNode() && tagNode != 0L && containsTagId(id))
    {
        d->tagIdToNodeMap.remove(id);
        emit signalTagNodeRemoved(tagNode);
    }
}

bool TagNodeList::containsTagId(const TQString& tagId)
{
    return d->tagIdToNodeMap.contains(tagId);
}

TQValueList<TagNode*> TagNodeList::toList() const
{
    return d->tagIdToNodeMap.values();
}

bool TagNodeList::readFromXML(const TQDomDocument& doc)
{
    return false; // TODO
}

TQDomDocument TagNodeList::toXML() const
{
    return TQDomDocument();
}

void TagNodeList::slotTagAdded(const Tag& tag)
{
    if (!containsTagId(tag.id()))
    {
        insert(new TagNode(tag, d->feedList->rootNode()));
    }
}

void TagNodeList::slotTagUpdated(const Tag& tag)
{
    if (containsTagId(tag.id()))
    {
        d->tagIdToNodeMap[tag.id()]->tagChanged();
    }
}
void TagNodeList::slotTagRemoved(const Tag& tag)
{
    if (containsTagId(tag.id()))
    {
        delete d->tagIdToNodeMap[tag.id()];
        d->tagIdToNodeMap[tag.id()] = 0;
    }
}

     
} // namespace Akregator

#include "tagnodelist.moc"
