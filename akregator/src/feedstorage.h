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
#ifndef AKREGATOR_BACKEND_FEEDSTORAGE_H
#define AKREGATOR_BACKEND_FEEDSTORAGE_H

#include <tqobject.h>
#include <tqstring.h>

#include "akregator_export.h"

class QStringList;


namespace Akregator {
namespace Backend {

/** a convenience class to handle categories in the backend */
class AKREGATOR_EXPORT Category 
{
    public:

    TQString term;
    TQString scheme;
    TQString name;

    /** two categories are equal when scheme and term are equal, name is ignored */

    bool operator==(const Category& other) const
    {
        return term == other.term && scheme == other.scheme;
    }

    bool operator!=(const Category& other) const 
    { 
        return !operator==(other); 
    }
    /** we need this for QMaps */
    bool operator<(const Category& other) const
    {
        return other.scheme < other.scheme || (other.scheme == other.scheme && term < other.term);
    }
};

class Storage;

class AKREGATOR_EXPORT FeedStorage : public QObject
{
    public:
    
        virtual int unread() = 0;
        virtual void setUnread(int unread) = 0;
        virtual int totalCount() = 0;
        virtual int lastFetch() = 0;
        virtual void setLastFetch(int lastFetch) = 0;
        
        /** returns the guids of all articles in this storage. If a tagID is given, only articles with this tag are returned */
        virtual TQStringList articles(const TQString& tagID=TQString::null) = 0;

        /** returns the guid of the articles in a given category */
        virtual TQStringList articles(const Category& cat) = 0;

        /** Appends all articles from another storage. If there is already an article in this feed with the same guid, it is replaced by the article from the source
        @param source the archive which articles should be appended
        */
        virtual void add(FeedStorage* source) = 0;

        /** reads an article from another storage and adds it to this storage */
        virtual void copyArticle(const TQString& guid, FeedStorage* source) = 0;

        /** deletes all articles from the archive */
        virtual void clear() = 0;

        
        virtual bool contains(const TQString& guid) = 0;
        virtual void addEntry(const TQString& guid) = 0;
        virtual void deleteArticle(const TQString& guid) = 0;
        virtual int comments(const TQString& guid) = 0;
        virtual TQString commentsLink(const TQString& guid) = 0;
        virtual void setCommentsLink(const TQString& guid, const TQString& commentsLink) = 0;
        virtual void setComments(const TQString& guid, int comments) = 0;
        virtual bool guidIsHash(const TQString& guid) = 0;
        virtual void setGuidIsHash(const TQString& guid, bool isHash) = 0;
        virtual bool guidIsPermaLink(const TQString& guid) = 0;
        virtual void setGuidIsPermaLink(const TQString& guid, bool isPermaLink) = 0;
        virtual uint hash(const TQString& guid) = 0;
        virtual void setHash(const TQString& guid, uint hash) = 0;
        virtual void setDeleted(const TQString& guid) = 0;
        virtual TQString link(const TQString& guid) = 0;
        virtual void setLink(const TQString& guid, const TQString& link) = 0;
        virtual uint pubDate(const TQString& guid) = 0;
        virtual void setPubDate(const TQString& guid, uint pubdate) = 0;
        virtual int status(const TQString& guid) = 0;
        virtual void setStatus(const TQString& guid, int status) = 0;
        virtual TQString title(const TQString& guid) = 0;
        virtual void setTitle(const TQString& guid, const TQString& title) = 0;
        virtual TQString description(const TQString& guid) = 0;
        virtual void setDescription(const TQString& guid, const TQString& description) = 0;

        virtual void addTag(const TQString& guid, const TQString& tag) = 0;
        virtual void removeTag(const TQString& guid, const TQString& tag) = 0;

        /** returns the tags of a given article. If @c guid is null, it returns all tags used in this feed */
        virtual TQStringList tags(const TQString& guid=TQString::null) = 0;

        virtual void addCategory(const TQString& guid, const Category& category) = 0;
        virtual TQValueList<Category> categories(const TQString& guid=TQString::null) = 0;

        virtual void setEnclosure(const TQString& guid, const TQString& url, const TQString& type, int length) = 0;
        virtual void removeEnclosure(const TQString& guid) = 0;
        
        virtual void setAuthor(const TQString& /*guid*/, const TQString& /*author*/) {}
        virtual TQString author(const TQString& /*guid*/) { return TQString(); }
        
        virtual void enclosure(const TQString& guid, bool& hasEnclosure, TQString& url, TQString& type, int& length) = 0;
        virtual void close() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;
    
        virtual void convertOldArchive() = 0;
};

}
}

#endif
