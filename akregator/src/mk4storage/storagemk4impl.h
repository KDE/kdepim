/*
    This file is part of Akregator.

    Copyright (C) 2005 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#ifndef STORAGEMK4IMPL_H
#define STORAGEMK4IMPL_H

#include "storage.h"

namespace Akregator {
namespace Backend {

/**
 * Metakit implementation of Storage interface
 */
class StorageMK4Impl : public Storage
{
    Q_OBJECT
    public:

        StorageMK4Impl();     
        StorageMK4Impl(const StorageMK4Impl&);
        StorageMK4Impl &operator =(const StorageMK4Impl&);
        virtual ~StorageMK4Impl();


        /** KGlobal::dirs()->saveLocation("data", "akregator")+"/Archive" */
        static QString defaultArchivePath();
        
        /** sets the directory where the metakit files will be stored.
            
            @param archivePath the path to the archive, or QString::null to reset it to the default.
         */   
        void setArchivePath(const QString& archivePath);

        /** returns the path to the metakit archives */
        QString archivePath() const;

        
        
        virtual void initialize(const QStringList& params);
        /**
         * Open storage and prepare it for work.
         * @return true on success.
         */
        virtual bool open(bool autoCommit = false);

        /**
         * Commit changes made in feeds and articles, making them persistent.
         * @return true on success.
         */
        virtual bool commit();

        /**
         * Rollback changes made in feeds and articles, reverting to last committed values.
         * @returns true on success.
         */
        virtual bool rollback();

        /**
         * Closes storage, freeing all allocated resources. Called from destructor, so you don't need to call it directly.
         * @return true on success.
         */
        virtual bool close();

        /**
         * @return Article archive for feed at given url.
         */
        virtual FeedStorage* archiveFor(const QString &url);
        virtual bool autoCommit() const;
        virtual int unreadFor(const QString &url);
        virtual void setUnreadFor(const QString &url, int unread);
        virtual int totalCountFor(const QString &url);
        virtual void setTotalCountFor(const QString &url, int total);
        virtual int lastFetchFor(const QString& url);
        virtual void setLastFetchFor(const QString& url, int lastFetch);
        
        virtual QStringList feeds() const;
       
        virtual void storeFeedList(const QString& opmlStr);
        virtual QString restoreFeedList() const;

        virtual void storeTagSet(const QString& xmlStr);
        virtual QString restoreTagSet() const; 
  
        /** adds all feed storages from a source to this storage
            existing articles are replaced
        */
        virtual void add(Storage* source);
        
        /** deletes all feed storages in this archive */
        virtual void clear();
        
        virtual bool taggingEnabled() const;
        
        void markDirty();

    protected slots:
        virtual void slotCommit();
        
    private:
        class StorageMK4ImplPrivate;
        StorageMK4ImplPrivate *d;
};

}
}

#endif // STORAGEMK4IMPL_H
