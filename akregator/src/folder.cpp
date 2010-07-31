/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2004-2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#include "article.h"
#include "folder.h"
#include "fetchqueue.h"
#include "treenodevisitor.h"

#include <tqlistview.h>
#include <tqdom.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>

#include <kdebug.h>

namespace Akregator {

class Folder::FolderPrivate
{
    public:
        /** List of children */
        TQValueList<TreeNode*> children;
        /** caching unread count of children */
        int unread;
        /** whether or not the folder is expanded */
        bool open;

        /** caches guids for notifying added articles */
        TQValueList<Article> addedArticlesNotify;
        /** caches guids for notifying removed articles */
        TQValueList<Article> removedArticlesNotify;
};
           
bool Folder::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitFolder(this))
        return true;
    else
        return visitor->visitTreeNode(this);
}

Folder* Folder::fromOPML(TQDomElement e)
{
    Folder* fg = new Folder(e.hasAttribute(TQString::fromLatin1("text")) ? e.attribute(TQString::fromLatin1("text")) : e.attribute(TQString::fromLatin1("title")));
    fg->setOpen( e.attribute(TQString::fromLatin1("isOpen")) != TQString::fromLatin1(("false")));
    fg->setId( e.attribute(TQString::fromLatin1("id")).toUInt() );
    return fg;
}

Folder::Folder(const TQString& title) : TreeNode(), d(new FolderPrivate)
{
    d->unread = 0;
    setTitle(title);
} 

Folder::~Folder()
{
    TreeNode* tmp = 0;
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != d->children.end(); ++it)
    {
        delete tmp;
        tmp = *it;
    }
    delete tmp;
    
    emitSignalDestroyed();

    delete d;
    d = 0;
}

TQStringList Folder::tags() const
{
    TQStringList t;
    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
    {
        // intersect tag sets instead of appending lists, to avoid dupes. This sucks. Definitely. I want QSet. Now.
        TQStringList t2 = (*it)->tags();
        for (TQStringList::ConstIterator it2 = t2.begin(); it2 != t2.end(); ++it2)
            if (!t.contains(*it2))
                t.append(*it2);
    } 
    return t;
}

TQValueList<Article> Folder::articles(const TQString& tag)
{
    TQValueList<Article> seq;
    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        seq += (*it)->articles(tag);
     
    return seq;
}

TQDomElement Folder::toOPML( TQDomElement parent, TQDomDocument document ) const
{
    TQDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    el.setAttribute("isOpen", d->open ? "true" : "false");
    el.setAttribute( "id", TQString::number(id()) );

    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        el.appendChild( (*it)->toOPML(el, document) );
        
    return el;
}

TQValueList<TreeNode*> Folder::children() const
{
    return d->children;
}

void Folder::insertChild(TreeNode* node, TreeNode* after)
{
    int pos = d->children.findIndex(after);
    
    if (pos < 0)
        prependChild(node);
    else 
        insertChild(pos+1, node);
}

void Folder::insertChild(uint index, TreeNode* node)
{
//    kdDebug() << "enter Folder::insertChild(int, node) " << node->title() << endl;
    if (node)
    {
        if (index >= d->children.size())
            d->children.append(node);
        else
            d->children.insert(d->children.at(index), node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified(); 
    }   
//    kdDebug() << "leave Folder::insertChild(int, node) " << node->title() << endl; 
}

void Folder::appendChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::appendChild() " << node->title() << endl;
    if (node)
    {
        d->children.append(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }    
//    kdDebug() << "leave Folder::appendChild() " << node->title() << endl;
}

void Folder::prependChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::prependChild() " << node->title() << endl;
    if (node)
    {
        d->children.prepend(node);
        node->setParent(this);
        connectToNode(node);
        updateUnreadCount();
        emit signalChildAdded(node);
        d->addedArticlesNotify += node->articles();
        articlesModified();
        nodeModified();
    }    
//    kdDebug() << "leave Folder::prependChild() " << node->title() << endl;
}

void Folder::removeChild(TreeNode* node)
{
//    kdDebug() << "enter Folder::removeChild() node:" << (node ? node->title() : "null") << endl;
    if (node && d->children.contains(node))
    {    
        node->setParent(0);
        d->children.remove(node);
        disconnectFromNode(node);
        updateUnreadCount();    
        emit signalChildRemoved(this, node);
        d->removedArticlesNotify += node->articles();
        articlesModified(); // articles were removed, TODO: add guids to a list
        nodeModified();
    }
//    kdDebug() << "leave Folder::removeChild() node: " << (node ? node->title() : "null") << endl;
}


TreeNode* Folder::firstChild()
{
    return d->children.isEmpty() ? 0 : d->children.first();
}            

TreeNode* Folder::lastChild()
{
    return d->children.isEmpty() ? 0 : d->children.last();
}
            
bool Folder::isOpen() const
{
    return d->open;
}

void Folder::setOpen(bool open)
{
    d->open = open;
}
            
int Folder::unread() const
{
    return d->unread;
}

int Folder::totalCount() const
{
    int totalCount = 0;

    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        totalCount += (*it)->totalCount();
    
    return totalCount;
}

void Folder::updateUnreadCount()
{
    int unread = 0;

    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        unread += (*it)->unread();
    
    d->unread = unread;
}

void Folder::slotMarkAllArticlesAsRead() 
{
    setNotificationMode(false);
    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotMarkAllArticlesAsRead();
    setNotificationMode(true, true);
}
    
void Folder::slotChildChanged(TreeNode* /*node*/)
{
    updateUnreadCount();
    nodeModified();
}

void Folder::slotChildDestroyed(TreeNode* node)
{
    d->children.remove(node);
    updateUnreadCount();    
    nodeModified();
}

void Folder::slotDeleteExpiredArticles()
{
    setNotificationMode(false);
    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotDeleteExpiredArticles();
    setNotificationMode(true, true);
}

void Folder::slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchOnly)
{
    TQValueList<TreeNode*>::ConstIterator en = d->children.end();
    for (TQValueList<TreeNode*>::ConstIterator it = d->children.begin(); it != en; ++it)
        (*it)->slotAddToFetchQueue(queue, intervalFetchOnly);
}

void Folder::doArticleNotification()
{
}

void Folder::connectToNode(TreeNode* child)
{
        connect(child, TQT_SIGNAL(signalChanged(TreeNode*)), this, TQT_SLOT(slotChildChanged(TreeNode*)));
        connect(child, TQT_SIGNAL(signalDestroyed(TreeNode*)), this, TQT_SLOT(slotChildDestroyed(TreeNode*)));
        connect(child, TQT_SIGNAL(signalArticlesAdded(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesAdded(TreeNode*, const TQValueList<Article>&)));
        connect(child, TQT_SIGNAL(signalArticlesRemoved(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesRemoved(TreeNode*, const TQValueList<Article>&)));
        connect(child, TQT_SIGNAL(signalArticlesUpdated(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesUpdated(TreeNode*, const TQValueList<Article>&)));
}

void Folder::disconnectFromNode(TreeNode* child)
{
        disconnect(child, TQT_SIGNAL(signalChanged(TreeNode*)), this, TQT_SLOT(slotChildChanged(TreeNode*)));
        disconnect(child, TQT_SIGNAL(signalDestroyed(TreeNode*)), this, TQT_SLOT(slotChildDestroyed(TreeNode*)));
        disconnect(child, TQT_SIGNAL(signalArticlesAdded(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesAdded(TreeNode*, const TQValueList<Article>&)));
        disconnect(child, TQT_SIGNAL(signalArticlesRemoved(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesRemoved(TreeNode*, const TQValueList<Article>&)));
        disconnect(child, TQT_SIGNAL(signalArticlesUpdated(TreeNode*, const TQValueList<Article>&)), this, TQT_SIGNAL(signalArticlesUpdated(TreeNode*, const TQValueList<Article>&)));
}
            
TreeNode* Folder::next()
{
    if ( firstChild() )
        return firstChild();

    if ( nextSibling() )
        return nextSibling();
    
    Folder* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

} // namespace Akregator
#include "folder.moc"
