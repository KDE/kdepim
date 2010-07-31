/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#ifndef FEEDSTORAGEMK4IMPL_H
#define FEEDSTORAGEMK4IMPL_H

#include "feedstorage.h"
namespace Akregator {
namespace Backend {

class StorageMK4Impl;
class FeedStorageMK4Impl : public FeedStorage
{
    public:
        FeedStorageMK4Impl(const TQString& url, StorageMK4Impl* main);
        virtual ~FeedStorageMK4Impl();


        virtual void add(FeedStorage* source);
        virtual void copyArticle(const TQString& guid, FeedStorage* source);
        virtual void clear();
        
        virtual int unread();
        virtual void setUnread(int unread);
        virtual int totalCount();
        virtual int lastFetch();
        virtual void setLastFetch(int lastFetch);

        virtual TQStringList articles(const TQString& tag=TQString::null);

        virtual TQStringList articles(const Category& cat);

        virtual bool contains(const TQString& guid);
        virtual void addEntry(const TQString& guid);
        virtual void deleteArticle(const TQString& guid);
        virtual int comments(const TQString& guid);
        virtual TQString commentsLink(const TQString& guid);
        virtual void setCommentsLink(const TQString& guid, const TQString& commentsLink);
        virtual void setComments(const TQString& guid, int comments);
        virtual bool guidIsHash(const TQString& guid);
        virtual void setGuidIsHash(const TQString& guid, bool isHash);
        virtual bool guidIsPermaLink(const TQString& guid);
        virtual void setGuidIsPermaLink(const TQString& guid, bool isPermaLink);
        virtual uint hash(const TQString& guid);
        virtual void setHash(const TQString& guid, uint hash);
        virtual void setDeleted(const TQString& guid);
        virtual TQString link(const TQString& guid);
        virtual void setLink(const TQString& guid, const TQString& link); 
        virtual uint pubDate(const TQString& guid);
        virtual void setPubDate(const TQString& guid, uint pubdate);
        virtual int status(const TQString& guid);
        virtual void setStatus(const TQString& guid, int status);
        virtual TQString title(const TQString& guid);
        virtual void setTitle(const TQString& guid, const TQString& title);
        virtual TQString description(const TQString& guid);
        virtual void setDescription(const TQString& guid, const TQString& description);
        virtual void setEnclosure(const TQString& guid, const TQString& url, const TQString& type, int length);
        virtual void removeEnclosure(const TQString& guid);
        virtual void enclosure(const TQString& guid, bool& hasEnclosure, TQString& url, TQString& type, int& length);
        
        virtual void addTag(const TQString& guid, const TQString& tag);
        virtual void removeTag(const TQString& guid, const TQString& tag);
        virtual TQStringList tags(const TQString& guid=TQString::null);

        virtual void addCategory(const TQString& guid, const Category& category);
        virtual TQValueList<Category> categories(const TQString& guid=TQString::null);

        virtual void setAuthor(const TQString& guid, const TQString& author);
        virtual TQString author(const TQString& guid);
        
        virtual void close();
        virtual void commit();
        virtual void rollback();
        
        virtual void convertOldArchive();
   private:
        void markDirty();
        /** finds article by guid, returns -1 if not in archive **/
        int findArticle(const TQString& guid);
        void setTotalCount(int total);
        class FeedStorageMK4ImplPrivate;
        FeedStorageMK4ImplPrivate* d;
};

}
}
#endif // FEEDSTORAGEMK4IMPL_H
