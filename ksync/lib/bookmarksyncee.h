/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef BOOKMARKSYNCEE_H
#define BOOKMARKSYNCEE_H

#include <qvaluelist.h>

#include <kbookmark.h>

#include "ksyncer.h"

class KBookmarkManager;

class BookmarkSyncEntry : public KSyncEntry
{
  public:
    BookmarkSyncEntry(KBookmark);

    QString name();
    QString id();
    QString timestamp();

    bool equals(KSyncEntry *entry);

    KBookmark bookmark() const { return mBookmark; }

  private:
    KBookmark mBookmark;
};

/**
  This class provides an implementation of the @see KSyncee interface for KSync.
  It provides syncing of bookmark files as used by Konqueror.
*/
class BookmarkSyncee : public KSyncee
{
  public:
    BookmarkSyncee();
    ~BookmarkSyncee();

    BookmarkSyncEntry *firstEntry();
    BookmarkSyncEntry *nextEntry();

//    BookmarkSyncEntry *findEntry(const QString &id);

    void addEntry(KSyncEntry *);
    void removeEntry(KSyncEntry *);

    bool read();
    bool write();

  private:
    BookmarkSyncEntry *createEntry(KBookmark);
    void listGroup(KBookmarkGroup);
    KBookmarkGroup findGroup(KBookmarkGroup group);

    KBookmarkManager *mBookmarkManager;
    QValueList<QDomElement> mBookmarks;
    QValueList<QDomElement>::ConstIterator mBookmarkIterator;
    QPtrList<BookmarkSyncEntry> mEntries;
};

#endif
