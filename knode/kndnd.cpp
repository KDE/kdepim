/*
    kndnd.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qdragobject.h>

#include "kndnd.h"
#include "kncollectionviewitem.h"
#include "kncollection.h"
#include "knmime.h"
#include "knhdrviewitem.h"

#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"


void KNArticleDragHandler::startDrag(QListViewItem *i)
{
  QDragObject *d=0;
  KNHdrViewItem *hvi=static_cast<KNHdrViewItem*>(i);
  if(hvi && hvi->art && hvi->isSelected()) {
    switch(hvi->art->type()) {
      case KNMimeBase::ATremote :
      case KNMimeBase::ATlocal :
        d=new QStoredDrag( "x-knode-drag/article" , w_idget);
        d->setPixmap(knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::posting), QPoint(-10,0));
      break;
      default:
      break;
    }
  }

  if(d)
    d->dragCopy();
}



// =================================================================================



void KNCollectionDragHandler::startDrag(QListViewItem *i)
{
  QDragObject *d=0;
  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(i);
  if(cvi && cvi->coll && cvi->coll->type()==KNCollection::CTfolder && cvi->isSelected()) {
    d=new QStoredDrag( "x-knode-drag/folder" , w_idget);
    d->setPixmap(knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::folder), QPoint(-10,0));
    d->dragCopy();
  }
}


bool KNCollectionDragHandler::accept(QDragEnterEvent *e)
{

  return ( e && (e->provides("x-knode-drag/article") || e->provides("x-knode-drag/folder")) );
}


bool KNCollectionDragHandler::accept(QDragMoveEvent *e, QListViewItem *i)
{
  if(!e) return false;

  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(i);

  return (
           (
             cvi && cvi->coll && cvi->coll->type()==KNCollection::CTfolder &&
             (e->provides("x-knode-drag/article") || e->provides("x-knode-drag/folder"))
           ) ||

           (
             !cvi && e->provides("x-knode-drag/folder")
           )
         );
}


bool KNCollectionDragHandler::accept(QDropEvent *e, QListViewItem *i)
{
  if(!e) return false;

  KNCollectionViewItem *cvi=static_cast<KNCollectionViewItem*>(i);

  return (
           (
             cvi && cvi->coll && cvi->coll->type()==KNCollection::CTfolder &&
             (e->provides("x-knode-drag/article") || e->provides("x-knode-drag/folder"))
           ) ||

           (
             !cvi && e->provides("x-knode-drag/folder")
           )
         );
}



