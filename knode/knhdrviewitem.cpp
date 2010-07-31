/*
    knhdrviewitem.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <tqdragobject.h>
#include <tqpainter.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kstringhandler.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knhdrviewitem.h"
#include "knarticle.h"
#include "headerview.h"


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
    TQListView *lv = listView();
    if (lv)
      static_cast<KNHeaderView*>( lv )->activeRemoved();
  }

  if (art) art->setListItem( 0 );
}


void KNHdrViewItem::expandChildren()
{
  TQListViewItemIterator it( firstChild() );
  for ( ; it.current(); ++it) {
    if (it.current()->depth() <= depth())
      break;
    it.current()->setOpen( true );
  }
}


int KNHdrViewItem::compare( TQListViewItem *i, int col, bool ) const
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


void KNHdrViewItem::paintCell( TQPainter *p, const TQColorGroup &cg, int column, int width, int alignment )
{
  int xText = 0, xPM = 3, yPM = 0;
  TQColor base;
  const KPaintInfo *paintInfo = static_cast<KNHeaderView*>( listView() )->paintInfo();

  TQPen pen = p->pen();
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

  p->fillRect( 0, 0, width, height(), TQBrush(base) );

  if ( column == paintInfo->subCol ) {
    TQFont font = p->font();
    font.setBold( firstColBold() );
    p->setFont( font );
    const TQPixmap *pm;

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
    TQString t2;
    TQFont f2;
    if (countUnreadInThread() > 0 && column == paintInfo->subCol && !isOpen()) {
      t2 = TQString( " (%1)" ).arg( countUnreadInThread() );
      f2 = p->font();
      f2.setBold( true );
      cntWidth = TQFontMetrics( f2 ).width( t2, -1 );
    }
    TQString t = KStringHandler::rPixelSqueeze( text( column ), p->fontMetrics(), width - xText - cntWidth - 5 );

    // show tooltip if we have to squeeze the text
    if ( t != text( column ) )
      mShowToolTip[column] = true;
    else
      mShowToolTip[column] = false;

    p->drawText( xText, 0, width - xText - 5, height(), alignment | AlignVCenter,  t );
    if (cntWidth) {
      TQFont orig = p->font();
      p->setFont( f2 );
      TQPen pen = p->pen();
      if (isSelected() || mActive) {
        pen.setColor( cg.highlightedText() );
      } else {
        pen.setColor( cg.link() );
      }
      p->setPen( pen );
      p->drawText( xText + TQFontMetrics( orig ).width( t, -1 ), 0, width - xText - 5, height(), alignment | AlignVCenter,  t2 );
    }
  }
}


int KNHdrViewItem::width( const TQFontMetrics &fm, const TQListView *, int column )
{
  int ret = fm.boundingRect( text(column) ).width();
  const KPaintInfo *paintInfo = static_cast<KNHeaderView*>( listView() )->paintInfo();

  // all pixmaps are drawn in the first column
  if ( column == paintInfo->subCol ) {
    const TQPixmap *pm;
    for (int i = 0; i < 4; ++i) {
      pm = pixmap( i );
      if (pm && !pm->isNull())
        ret += pm->width() + 3;
    }
  }

  return ret;
}


TQString KNHdrViewItem::text( int col ) const
{
  if ( !art )
    return TQString::null;
  KNHeaderView *hv = static_cast<KNHeaderView*>( listView() );

  if ( col == hv->paintInfo()->subCol ) {
    return art->subject()->asUnicodeString();
  }

  if ( col == hv->paintInfo()->sizeCol ) {
    if ( art->lines()->numberOfLines() != -1 )
      return TQString::number( art->lines()->numberOfLines() );
    else
      return TQString::null;
  }

  if ( col == hv->paintInfo()->scoreCol ) {
    if ( art->type() == KMime::Base::ATremote )
      return TQString::number( static_cast<KNRemoteArticle*>( art )->score() );
    else
      return TQString::null;
  }

  if ( col == hv->paintInfo()->dateCol ) {
    return hv->mDateFormatter.dateString( art->date()->qdt() );
  } else
    return KListViewItem::text( col );
}


TQDragObject* KNHdrViewItem::dragObject()
{
  TQDragObject *d = new TQStoredDrag( "x-knode-drag/article" , listView()->viewport() );
  d->setPixmap( knGlobals.configManager()->appearance()->icon( KNConfig::Appearance::posting ) );
  return d;
}


int KNHdrViewItem::countUnreadInThread()
{
  int count = 0;
  if (knGlobals.configManager()->readNewsGeneral()->showUnread()) {
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


TQColor KNHdrViewItem::normalColor()
{
  if (art->type()==KMime::Base::ATremote)
    return static_cast<KNRemoteArticle*>( art )->color();
  else
    return knGlobals.configManager()->appearance()->unreadThreadColor();
}


TQColor KNHdrViewItem::greyColor()
{
  return knGlobals.configManager()->appearance()->readThreadColor();
}

