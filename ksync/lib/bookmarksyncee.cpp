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

#include <kdebug.h>

#include <kbookmarkmanager.h>

#include "bookmarksyncee.h"

BookmarkSyncEntry::BookmarkSyncEntry(KBookmark bm) :
  mBookmark(bm)
{
}

QString BookmarkSyncEntry::name()
{
  return mBookmark.text();
}

QString BookmarkSyncEntry::id()
{
  return mBookmark.url().url();
}

QString BookmarkSyncEntry::timestamp()
{
  return mBookmark.text() + mBookmark.url().url();
}

bool BookmarkSyncEntry::equals(KSyncEntry *entry)
{
  BookmarkSyncEntry *bmEntry = dynamic_cast<BookmarkSyncEntry *>(entry);
  if (!bmEntry) {
    kdDebug() << "BookmarkSyncee::addEntry(): Wrong type." << endl;
    return false;
  }

  KBookmark bm = bmEntry->bookmark();

  kdDebug() << "equals: '" << mBookmark.fullText() << "' <-> '"
            << bm.fullText() << "'" << endl;

  if (mBookmark.fullText() != bmEntry->bookmark().fullText()) return false;
  if (mBookmark.url() != bmEntry->bookmark().url()) return false;
  // TODO: Compare grouping
  
  return true;
}


BookmarkSyncee::BookmarkSyncee()
{
  mBookmarkManager = 0;

  mEntries.setAutoDelete(true);
}

BookmarkSyncee::~BookmarkSyncee()
{
  delete mBookmarkManager;
}

bool BookmarkSyncee::read()
{
  delete mBookmarkManager;
  mBookmarkManager = KBookmarkManager::managerForFile( filename() );
  
  mBookmarks.clear();
  
  listGroup(mBookmarkManager->root());

  mBookmarkIterator = mBookmarks.begin();

  return true;
}

void BookmarkSyncee::listGroup(KBookmarkGroup group)
{
  for(KBookmark bm = group.first(); !bm.isNull(); bm = group.next(bm)) {
    if (bm.isGroup()) {
      listGroup(bm.toGroup());
    } else if (bm.isSeparator()) {
      // Skip separators for now, but these should be synced, too.
    } else {
      kdDebug() << "appending '" << bm.text() << "' ("
                << bm.parentGroup().fullText() << ")" << endl;
      mBookmarks.append(bm.internalElement());
    }
  }
}

bool BookmarkSyncee::write()
{
  mBookmarkManager->save();

  return true;
}


BookmarkSyncEntry *BookmarkSyncee::firstEntry()
{
  mBookmarkIterator = mBookmarks.begin();
  return createEntry(KBookmark(*mBookmarkIterator));
}

BookmarkSyncEntry *BookmarkSyncee::nextEntry()
{
  return createEntry(KBookmark(*(++mBookmarkIterator)));
}

#if 0
BookmarkSyncEntry *BookmarkSyncee::findEntry(const QString &id)
{
  QValueList<QDomElement>::Iterator bmIt = mBookmarks.begin();
  while (bmIt != mBookmarks.end()) {
    if (KBookmark(*bmIt).url().url() == id) {
      return createEntry(KBookmark(*bmIt));
    }
    ++bmIt;
  }

  return 0;
}
#endif

void BookmarkSyncee::addEntry(KSyncEntry *entry)
{
  BookmarkSyncEntry *bmEntry = dynamic_cast<BookmarkSyncEntry *>(entry);
  if (!bmEntry) {
    kdDebug() << "BookmarkSyncee::addEntry(): Wrong type." << endl;
  } else {
    KBookmark bm = bmEntry->bookmark();
    KBookmarkGroup bmGroup = findGroup(bm.parentGroup());
    KBookmark newBookmark = bmGroup.addBookmark( mBookmarkManager,
                                                 bm.fullText(), bm.url() );
    mBookmarks.append(newBookmark.internalElement());
  }
}

void BookmarkSyncee::removeEntry(KSyncEntry *entry)
{
  BookmarkSyncEntry *bmEntry = dynamic_cast<BookmarkSyncEntry *>(entry);
  if (!bmEntry) {
    kdDebug() << "BookmarkSyncee::addEntry(): Wrong type." << endl;
  } else {
    KBookmark bm = bmEntry->bookmark();
    kdDebug() << "Remove " << bm.text() << endl;
    // TODO: implement
/*
    KBookmarkGroup bmGroup = findGroup(bm.parentGroup());
    KBookmark newBookmark = bmGroup.addBookmark(bm.fullText(),bm.url());
    mBookmarks.append(newBookmark.internalElement());
*/
  }
}

KBookmarkGroup BookmarkSyncee::findGroup(KBookmarkGroup group)
{
  if (group.fullText().isEmpty()) return mBookmarkManager->root();

  QValueList<QDomElement>::Iterator bmIt = mBookmarks.begin();
  while (bmIt != mBookmarks.end()) {
    KBookmark bm(*bmIt);
    if (bm.isGroup() && (bm.fullText() == group.fullText())) {
      return bm.toGroup();
    }
    ++bmIt;
  }
  KBookmarkGroup newGroup =
      mBookmarkManager->root().createNewFolder( mBookmarkManager, 
                                                group.fullText() );
  mBookmarks.append(newGroup.internalElement());

  return newGroup;
}

BookmarkSyncEntry *BookmarkSyncee::createEntry(KBookmark bm)
{
  if (!bm.isNull()) {
    BookmarkSyncEntry *entry = new BookmarkSyncEntry(bm);
    mEntries.append(entry);
    return entry;    
  } else {
    return 0;
  }
}
