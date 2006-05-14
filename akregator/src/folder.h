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

#ifndef AKREGATOR_FOLDER_H
#define AKREGATOR_FOLDER_H

#include "treenode.h"

class QDomDocument;
class QDomElement;
class QStringList;
template <class T> class QValueList;

namespace Akregator
{
    class Article;
    class FetchQueue;
    class TreeNodeVisitor;
        
    /** Represents a folder (containing feeds and/or other folders)
     */
    class Folder : public TreeNode
    {
        Q_OBJECT
        public:
            /** creates a feed group parsed from a XML dom element.
            Child nodes are not inserted or parsed. 
            @param e the element representing the feed group
            @return a freshly created feed group */
            static Folder* fromOPML(QDomElement e);
            
            /** Creates a new folder with a given title
            @param title The title of the feed group
             */          
            Folder(const QString& title = QString::null);
            
            virtual ~Folder();

            virtual bool accept(TreeNodeVisitor* visitor);
            
            /** returns recursively concatenated articles of children  
            @return an article sequence containing articles of children */
            virtual QValueList<Article> articles(const QString& tag=QString::null);

            /** returns a list of all tags occurring in the subtree of this folder */
            virtual QStringList tags() const;
            
            /** returns the number of unread articles in all children    
            @return number of unread articles */
            virtual int unread() const;
            
            /** returns the number of articles in all children
            @return number of articles */
            virtual int totalCount() const;
            
            /** Helps the rest of the app to decide if node should be handled as group or not. Use only where necessary, use polymorphism where possible. */
            virtual bool isGroup() const { return true; }

            /** converts the feed group into OPML format for save and export and appends it to node @c parent in document @document.
            Children are processed and appended recursively.
            @param parent The parent element 
            @param document The DOM document 
            @return The newly created element representing this feed group */
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;
            
            /** returns the (direct) children of this node.
            @return a list of pointers to the child nodes
             */
            virtual QValueList<TreeNode*> children() const;
            
            /** inserts @c node as child after child node @c after.
            if @c after is not a child of this group, @c node will be inserted as first child
            @param node the tree node to insert
            @param after the node after which @c node will be inserted */
            virtual void insertChild(TreeNode* node, TreeNode* after);
            
            /** inserts @c node as first child
            @param node the tree node to insert */
            virtual void prependChild(TreeNode* node);

            /** inserts @c node as last child
            @param node the tree node to insert */
            virtual void appendChild(TreeNode* node);

            /** remove @c node from children. Note that @c node will not be deleted
            @param node the child node to remove  */
            virtual void removeChild(TreeNode* node);

            /** returns the first child of the group, 0 if none exist */
            virtual TreeNode* firstChild();
            
            /** returns the last child of the group, 0 if none exist */
            virtual TreeNode* lastChild();
            
            /** returns whether the feed group is opened or not..
            Use only in \ref FolderItem. */
            virtual bool isOpen() const;
            
            /** open/close the feed group (display it as expanded/collapsed in the tree view). Use only in \ref FolderItem. */
            virtual void setOpen(bool open);
            
        signals:
            /** emitted when a child was added */
            void signalChildAdded(TreeNode*);

            /** emitted when a child was removed */
            void signalChildRemoved(Folder*, TreeNode*);
                       
        public slots:
            
            /** Delete expired articles recursively. */
            virtual void slotDeleteExpiredArticles();
            
            /** Mark articles of children recursively as read. */
            virtual void slotMarkAllArticlesAsRead();
 
            /** Called when a child was modified. 
            @param node the child that was changed
             */
            virtual void slotChildChanged(TreeNode* node);
            
            /** Called when a child was destroyed. 
            @param node the child that was destroyed
            */
            virtual void slotChildDestroyed(TreeNode* node);

            /** enqueues children recursively for fetching
            @param queue a fetch queue 
            @param internvalFetchesOnly */
            virtual void slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchesOnly=false);

            /** returns the next node in the tree.
            Calling next() unless it returns 0 iterates through the tree in pre-order
             */
            virtual TreeNode* next();

        protected:
        
            /** inserts @c node as child on position @c index
            @param index the position where to insert
            @param node the tree node to insert */
            virtual void insertChild(uint index, TreeNode* node);

            virtual void doArticleNotification();
        private:

            void connectToNode(TreeNode* child);
            void disconnectFromNode(TreeNode* child);
            
            virtual void updateUnreadCount();
            
            class FolderPrivate;
            FolderPrivate* d;
    };
}

#endif // AKREGATOR_FOLDER_H
