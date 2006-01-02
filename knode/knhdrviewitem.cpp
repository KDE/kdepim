/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <q3dragobject.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>

#include <kdeversion.h>
#include <kdebug.h>
#include <k3multipledrag.h>
#include <kstringhandler.h>
#include <k3urldrag.h>

#include <libkdepim/kdepimprotocols.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knhdrviewitem.h"
#include "knarticle.h"
#include "headerview.h"
#include "settings.h"


KNHdrViewItem::KNHdrViewItem( KNHeaderView *ref, KNArticle *a ) :
  KListViewItem( ref )
{
  init( a );
}


KNHdrViewItem::KNHdrViewItem( KNHdrViewItem *ref, KNArticle *a ) :
  KListViewItem( ref )
{
  init( a );
}


void KNHdrViewItem::init( KNArticle *a )
{
  art = a;
  mActive = false;
  for ( int i = 0; i < 5; ++i) // FIXME hardcoded column count
    mShowToolTip[i] = false;
}


KNHdrViewItem::~KNHdrViewItem()
{
  if (mActive) {
    Q3ListView *lv = listView();
    if (lv)
      static_cast<KNHeaderView*>( lv )->activeRemoved();
  }

  if (art) art->setListItem( 0 );
}


void KNHdrViewItem::expandChildren()
{
  Q3ListViewItemIterator it( firstChild() );
  for ( ; it.current(); ++it) {
    if (it.current()->depth() <= depth())
      break;
    it.current()->setOpen( true );
  }
}


int KNHdrViewItem::compare( Q3ListViewItem *i, int col, bool ) const
{
  KNArticle *otherArticle = static_cast<KNHdrViewItem*>( i )->art;
  int diff = 0;
  time_t date1 = 0, date2 = 0;

  switch (col) {
    case 0:
    case 1:
       return text( col ).localeAwareCompare( i->text(col) );

    case 2:
       if (art->type() == KMime::Base::ATremote) {
         diff = static_cast<KNRemoteArticle*>( art )->score() - static_cast<KNRemoteArticle*>( otherArticle )->score();
         return (diff < 0 ? -1 : diff > 0 ? 1 : 0);
       } else
         return 0;

    case 3:
       diff = art->lines()->numberOfLines() - otherArticle->lines()->numberOfLines();
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    case 4:
       date1 = art->date()->unixTime();
       date2 = otherArticle->date()->unixTime();
       if (art->type() == KMime::Base::ATremote && static_cast<KNHeaderView*>( listView() )->sortByThreadChangeDate()) {
         if (static_cast<KNRemoteArticle*>( art )->subThreadChangeDate() > date1)
           date1 = static_cast<KNRemoteArticle*>( art )->subThreadChangeDate();
         if (static_cast<KNRemoteArticle*>( otherArticle )->subThreadChangeDate() > date2)
           date2 = static_cast<KNRemoteArticle*>( otherArticle )->subThreadChangeDate();
       }
       diff = date1 - date2;
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    default:
       return 0;
  }
}


void KNHdrViewItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
  int xText = 0, xPM = 3, yPM = 0;
  QColor base;
  const KPaintInfo *paintInfo = static_cast<KNHeaderView*>( listView() )->paintInfo();

  QPen pen = p->pen();
  if (isSelected() || mActive) {
    pen.setColor( cg.highlightedText() );
    base = cg.highlight();
  } else {
    if (greyOut())
      pen.setColor( greyColor() );
    else
      pen.setColor( normalColor() );
    base = backgroundColor( column );
  }

  p->setPen( pen );

  p->fillRect( 0, 0, width, height(), QBrush(base) );

  if ( column == paintInfo->subCol ) {
    QFont font = p->font();
    font.setBold( firstColBold() );
    p->setFont( font );
    const QPixmap *pm;

    for (int i = 0; i < 4; i++) {
      pm = pixmap( i );
      if (pm && !pm->isNull()) {
        yPM = (height() - pm->height()) / 2;
        p->drawPixmap( xPM, yPM, *pm );
        xPM += pm->width() + 3;
      }
    }

    xText = xPM;
  }

  if (width - xText - 5 > 0) {
    int cntWidth = 0;
    QString t2;
    QFont f2;
    if (countUnreadInThread() > 0 && column == paintInfo->subCol && !isOpen()) {
      t2 = QString( " (%1)" ).arg( countUnreadInThread() );
      f2 = p->font();
      f2.setBold( true );
      cntWidth = QFontMetrics( f2 ).width( t2, -1 );
    }
    QString t = KStringHandler::rPixelSqueeze( text( column ), p->fontMetrics(), width - xText - cntWidth - 5 );

    // show tooltip if we have to squeeze the text
    if ( t != text( column ) )
      mShowToolTip[column] = true;
    else
      mShowToolTip[column] = false;

    p->drawText( xText, 0, width - xText - 5, height(), alignment | Qt::AlignVCenter,  t );
    if (cntWidth) {
      QFont orig = p->font();
      p->setFont( f2 );
      QPen pen = p->pen();
      if (isSelected() || mActive) {
        pen.setColor( cg.highlightedText() );
      } else {
        pen.setColor( cg.link() );
      }
      p->setPen( pen );
      p->drawText( xText + QFontMetrics( orig ).width( t, -1 ), 0, width - xText - 5, height(), alignment | Qt::AlignVCenter,  t2 );
    }
  }
}


int KNHdrViewItem::width( const QFontMetrics &fm, const Q3ListView *, int column )
{
  int ret = fm.boundingRect( text(column) ).width();
  const KPaintInfo *paintInfo = static_cast<KNHeaderView*>( listView() )->paintInfo();

  // all pixmaps are drawn in the first column
  if ( column == paintInfo->subCol ) {
    const QPixmap *pm;
    for (int i = 0; i < 4; ++i) {
      pm = pixmap( i );
      if (pm && !pm->isNull())
        ret += pm->width() + 3;
    }
  }

  return ret;
}


QString KNHdrViewItem::text( int col ) const
{
  if ( !art )
    return QString();
  KNHeaderView *hv = static_cast<KNHeaderView*>( listView() );

  if ( col == hv->paintInfo()->subCol ) {
    return art->subject()->asUnicodeString();
  }

  if ( col == hv->paintInfo()->sizeCol ) {
    if ( art->lines()->numberOfLines() != -1 )
      return QString::number( art->lines()->numberOfLines() );
    else
      return QString();
  }

  if ( col == hv->paintInfo()->scoreCol ) {
    if ( art->type() == KMime::Base::ATremote )
      return QString::number( static_cast<KNRemoteArticle*>( art )->score() );
    else
      return QString();
  }

  if ( col == hv->paintInfo()->dateCol ) {
    return hv->mDateFormatter.dateString( art->date()->qdt() );
  } else
    return KListViewItem::text( col );
}


Q3DragObject* KNHdrViewItem::dragObject()
{
  K3MultipleDrag *d = new K3MultipleDrag( listView()->viewport() );
  KURL::List list;
  QString mid = art->messageID()->asUnicodeString();
  // for some obscure reason it returns messageid in <>s
  mid = mid.mid( 1, mid.length() - 2 );
  list.append( KDEPIMPROTOCOL_NEWSARTICLE + mid );
  QMap<QString,QString> metadata;
  metadata["labels"] = KURL::encode_string( art->subject()->asUnicodeString() );
  d->addDragObject( new K3URLDrag( list, metadata, 0L ) );
  d->addDragObject( new Q3StoredDrag( "x-knode-drag/article" , 0L ) );
  d->setPixmap( knGlobals.configManager()->appearance()->icon( KNode::Appearance::posting ) );
  return d;
}


int KNHdrViewItem::countUnreadInThread()
{
  int count = 0;
  if ( knGlobals.settings()->showUnread() ) {
    if (art->type() == KMime::Base::ATremote) {
      count = static_cast<KNRemoteArticle*>( art )->unreadFollowUps();
    }
  }
  return count;
}


bool KNHdrViewItem::greyOut()
{
  if (art->type() == KMime::Base::ATremote) {
    return !static_cast<KNRemoteArticle*>( art )->hasUnreadFollowUps()
        && static_cast<KNRemoteArticle*>( art )->isRead();
  } else
    return false;
}


bool KNHdrViewItem::firstColBold()
{
  if(art->type() == KMime::Base::ATremote)
    return static_cast<KNRemoteArticle*>( art )->isNew();
  else
    return false;
}


QColor KNHdrViewItem::normalColor()
{
  if (art->type()==KMime::Base::ATremote)
    return static_cast<KNRemoteArticle*>( art )->color();
  else
    return knGlobals.settings()->unreadThreadColor();
}


QColor KNHdrViewItem::greyColor()
{
  return knGlobals.settings()->readThreadColor();
}

