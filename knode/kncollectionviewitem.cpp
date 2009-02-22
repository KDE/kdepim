/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <QDropEvent>

#include <kiconloader.h>


#include "kncollectionviewitem.h"
#include "kncollectionview.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knglobals.h"
#include "knconfigmanager.h"


KNCollectionViewItem::KNCollectionViewItem( KFolderTree *parent, Protocol protocol, Type type) :
  KFolderTreeItem(parent, QString(), protocol, type), coll(0)
{
  setIcon();
}


KNCollectionViewItem::KNCollectionViewItem( KFolderTreeItem *it, Protocol protocol, Type type, int unread, int total ) :
  KFolderTreeItem(it, QString(), protocol, type, unread, total), coll(0)
{
  setIcon();
}


KNCollectionViewItem::~KNCollectionViewItem()
{
  if(coll) coll->setListItem(0);
}


void KNCollectionViewItem::setIcon() {
  if ( protocol() == KFolderTreeItem::News ) {
    // news servers/groups
    switch ( type() ) {
      case KFolderTreeItem::Root:
        setPixmap( 0, SmallIcon("network-server") );
        break;
      default:
        setPixmap( 0, UserIcon("group") );
    }
  } else {
    // local folders
    switch ( type() ) {
      case KFolderTreeItem::Outbox:
        setPixmap( 0, SmallIcon("mail-folder-outbox") );
        break;
      case KFolderTreeItem::Drafts:
        setPixmap( 0, SmallIcon("document-properties") );
        break;
      case KFolderTreeItem::SentMail:
        setPixmap( 0, SmallIcon("mail-folder-sent") );
        break;
      default:
        setPixmap( 0, SmallIcon("folder") );
    }
  }
}


int KNCollectionViewItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
  KFolderTreeItem *other = static_cast<KFolderTreeItem*>(i);

  // folders should be always on the bottom
  if (protocol() == KFolderTreeItem::Local) {
    if (other && other->protocol() == KFolderTreeItem::News)
      return ascending ? 1 : -1;
  }

  // news servers should be always on top
  if (protocol() == KFolderTreeItem::News) {
    if (other && other->protocol() == KFolderTreeItem::Local)
      return ascending ? -1 : 1;
  }

  return KFolderTreeItem::compare(i, col, ascending);
}


bool KNCollectionViewItem::acceptDrag(QDropEvent* event) const
{
  const QMimeData *md = event?event->mimeData():0;
  if (md && coll && coll->type()==KNCollection::CTfolder) {
    if ( md->hasFormat("x-knode-drag/article") )
      return !(static_cast<KNFolder*>(coll)->isRootFolder());   // don't drop articles on the root folder
    else if (md->hasFormat("x-knode-drag/folder"))
      return !isSelected();             // don't drop on itself
  }
  return false;
}

QString KNCollectionViewItem::squeezeFolderName( const QString &text,
                                                 const QFontMetrics &fm,
                                                 uint width ) const
{
  if (protocol() == KFolderTreeItem::News && type() == KFolderTreeItem::Other) {
    QString t(text);
    int curPos = 0, nextPos = 0;
    QString temp;
    while ( (uint)fm.width(t) > width && nextPos != -1 ) {
      nextPos = t.indexOf( '.', curPos );
      if ( nextPos != -1 ) {
        temp = t[curPos];
        t.replace( curPos, nextPos - curPos, temp );
        curPos += 2;
      }
    }
    if ( (uint)fm.width( t ) > width )
      t = fm.elidedText( t, Qt::ElideRight, width  );
    return t;
  } else
    return KFolderTreeItem::squeezeFolderName( text, fm, width );
}
