/*
    kncollectionviewitem.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdragobject.h>
#include <qpainter.h>

#include <kiconloader.h>

#include "kncollectionviewitem.h"
#include "kncollectionview.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knglobals.h"
#include "knconfigmanager.h"


KNCollectionViewItem::KNCollectionViewItem( KFolderTree *parent, Protocol protocol, Type type) :
  KFolderTreeItem(parent, QString::null, protocol, type), coll(0)
{
  setIcon();
}


KNCollectionViewItem::KNCollectionViewItem( KFolderTreeItem *it, Protocol protocol, Type type, int unread, int total ) :
  KFolderTreeItem(it, QString::null, protocol, type, unread, total), coll(0)
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
        setPixmap( 0, SmallIcon("server") );
        break;
      default:
        setPixmap( 0, UserIcon("group") );
    }
  } else {
    // local folders
    switch ( type() ) {
      case KFolderTreeItem::Outbox:
        setPixmap( 0, SmallIcon("folder_outbox") );
        break;
      case KFolderTreeItem::Drafts:
        setPixmap( 0, SmallIcon("edit") );
        break;
      case KFolderTreeItem::SentMail:
        setPixmap( 0, SmallIcon("folder_sent_mail") );
        break;
      default:
        setPixmap( 0, SmallIcon("folder") );
    }
  }
}


int KNCollectionViewItem::compare(QListViewItem *i, int col, bool ascending) const
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
  if (event && coll && coll->type()==KNCollection::CTfolder) {
    if (event->provides("x-knode-drag/article"))
      return !(static_cast<KNFolder*>(coll)->isRootFolder());   // don't drop articles on the root folder
    else if (event->provides("x-knode-drag/folder"))
      return !isSelected();             // don't drop on itself
  }
  return false;
}


void KNCollectionViewItem::paintCell( QPainter * p, const QColorGroup & cg,int column, int width, int align )
{
  KFolderTree *ft = static_cast<KFolderTree*>(listView());
  QString curText = text(column);

  // find out if we will use bold font, necessary for the text squeezing
  if ( (column == 0 || column == ft->unreadIndex()) && ( mUnread > 0 ) ) {
    QFont f = p->font();
    f.setWeight(QFont::Bold);
    p->setFont(f);
  }

  // consider pixmap size for squeezing
  int pxWidth = 8;
  const QPixmap *px = pixmap(column);
  if (px)
    pxWidth += px->width();

  // temporary set the squeezed text and use the parent class to paint it
  setText( column, shortString(curText, column, width - pxWidth, p->fontMetrics()));
  KFolderTreeItem::paintCell( p, cg, column, width, align );
  setText( column, curText );
}


QString KNCollectionViewItem::shortString( const QString &text, int /*col*/, int width, QFontMetrics fm )
{
  if (protocol() == KFolderTreeItem::News && type() == KFolderTreeItem::Other) {
    QString t(text);
    int curPos = 0, nextPos = 0;
    QString temp;
    while ( (fm.width(t) > width) && (nextPos != -1) ) {
      nextPos = t.find('.', curPos);
      if ( nextPos != -1 ) {
        temp = t[curPos];
        t.replace( curPos, nextPos - curPos, temp );
        curPos += 2;
      }
    }
    return t;
  } else
    return text;
}
