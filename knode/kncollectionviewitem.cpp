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

#include "kncollectionviewitem.h"

#include "kncollectionview.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knglobals.h"
#include "knconfigmanager.h"

#include <kiconloader.h>

#include <QDropEvent>

KNCollectionViewItem::KNCollectionViewItem( FolderTreeWidget *parent, Protocol protocol, FolderType type) :
  FolderTreeWidgetItem(parent, QString(), protocol, type), coll(0)
{
  setUp();
}


KNCollectionViewItem::KNCollectionViewItem( FolderTreeWidgetItem *parent, Protocol protocol, FolderType type, int unread, int total ) :
  FolderTreeWidgetItem(parent, QString(), protocol, type), coll(0)
{
  setUp();
  setUnreadCount( unread );
  setTotalCount( total );
}


KNCollectionViewItem::~KNCollectionViewItem()
{
  if(coll) coll->setListItem(0);
}


void KNCollectionViewItem::setUp()
{
  // Label edition
  setFlags( flags() | Qt::ItemIsEditable );

  // Icons
  if ( protocol() == FolderTreeWidgetItem::News ) {
    // news servers/groups
    switch ( folderType() ) {
      case FolderTreeWidgetItem::Root:
        setIcon( 0, KIcon("network-server") );
        break;
      default:
        setIcon( 0, KIcon("group") );
    }
  } else {
    // local folders
    switch ( folderType() ) {
      case FolderTreeWidgetItem::Outbox:
        setIcon( 0, KIcon("mail-folder-outbox") );
        break;
      case FolderTreeWidgetItem::Drafts:
        setIcon( 0, KIcon("document-properties") );
        break;
      case FolderTreeWidgetItem::SentMail:
        setIcon( 0, KIcon("mail-folder-sent") );
        break;
      default:
        setIcon( 0, KIcon("folder") );
    }
  }
}


bool KNCollectionViewItem::operator<( const QTreeWidgetItem &other ) const
{
  const FolderTreeWidgetItem &otherFolder = static_cast<const FolderTreeWidgetItem&>( other );

  if( protocol() == FolderTreeWidgetItem::Local ) {
    if( otherFolder.protocol() == FolderTreeWidgetItem::News) {
      return false;
    }
  }

  if( protocol() == FolderTreeWidgetItem::News ) {
    if( otherFolder.protocol() == FolderTreeWidgetItem::Local ) {
      return true;
    }
  }
  return FolderTreeWidgetItem::operator<( other );
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


QString KNCollectionViewItem::elidedLabelText( const QFontMetrics &fm, unsigned int width ) const
{
  if (protocol() == FolderTreeWidgetItem::News && folderType() == FolderTreeWidgetItem::Other) {
    QString t( labelText() );
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
  } else {
    return FolderTreeWidgetItem::elidedLabelText( fm, width );
  }
}
