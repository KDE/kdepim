#ifndef BOOKMARKSYNCEE_H
#define BOOKMARKSYNCEE_H
// $Id$

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
  This class provides an implementation of the @KSyncee interface for KSync. It
  provides syncing of bookmark files as used by Konqueror.
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
    QList<BookmarkSyncEntry> mEntries;
};

#endif
