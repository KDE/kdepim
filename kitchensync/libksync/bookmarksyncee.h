/*
    This file is part of ksync.

    Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_BOOKMARKSYNCEE_H
#define KSYNC_BOOKMARKSYNCEE_H

#include "syncee.h"

#include <kbookmarkmanager.h>

#include <qvaluelist.h>
#include <qmap.h>

class KBookmarkManager;

namespace KSync {

class BookmarkSyncEntry : public SyncEntry
{
    friend class BookmarkSyncee;
  public:
    BookmarkSyncEntry( KBookmark , Syncee *parent );
    BookmarkSyncEntry( Syncee* parent );

    QString name();
    QString id();
    QString timestamp();

    bool equals( SyncEntry *entry );

    BookmarkSyncEntry *clone();

    KBookmark bookmark() const { return mBookmark; }

  private:
    void setBookmark( const KBookmark& );
    KBookmark mBookmark;
};

/**
  This class provides an implementation of the Syncee interface for libksync. It
  provides syncing of bookmark files as used by Konqueror.
*/
class BookmarkSyncee : public Syncee
{
  public:
    BookmarkSyncee(Merger * m= 0);
    BookmarkSyncee( KBookmarkManager *, Merger* m = 0);
    ~BookmarkSyncee();


    BookmarkSyncEntry *firstEntry();
    BookmarkSyncEntry *nextEntry();

//    BookmarkSyncEntry *findEntry( const QString &id );

    void addEntry( SyncEntry * );
    void removeEntry( SyncEntry * );

    bool writeBackup( const QString & );
    bool restoreBackup( const QString & );

  private:
    void init();

    BookmarkSyncEntry *createEntry( KBookmark );
    /* ### naming */
    void listGroup( KBookmarkGroup );
    KBookmarkGroup findGroup( KBookmarkGroup group );

    KBookmarkManager *mBookmarkManager;
    bool mOwnBookmarkManager;
    QValueList<QDomElement> mBookmarks;
    QValueList<QDomElement>::ConstIterator mBookmarkIterator;
    QMap<QString, BookmarkSyncEntry*> mEntries;
};

}

#endif
