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

#include <QDrag>
#include <QPainter>
#include <QPixmap>

#include <kdebug.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knhdrviewitem.h"
#include "knarticle.h"
#include "headerview.h"
#include "settings.h"


KNHdrViewItem::KNHdrViewItem( KNHeaderView *ref, KNArticle::Ptr a ) :
  K3ListViewItem( ref )
{
  init( a );
}


KNHdrViewItem::KNHdrViewItem( KNHdrViewItem *ref, KNArticle::Ptr a ) :
  K3ListViewItem( ref )
{
  init( a );
}


void KNHdrViewItem::init( KNArticle::Ptr a )
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

  if (art) {
    art->setListItem( 0, art );
  }
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
  KNArticle::Ptr otherArticle = static_cast<KNHdrViewItem*>( i )->art;
  int diff = 0;
  time_t date1 = 0, date2 = 0;

  switch (col) {
    case 0:
    case 1:
       return text( col ).localeAwareCompare( i->text(col) );

    case 2:
       if (art->type() == KNArticle::ATremote) {
         diff = boost::static_pointer_cast<KNRemoteArticle>( art )->score() - boost::static_pointer_cast<KNRemoteArticle>( otherArticle )->score();
         return (diff < 0 ? -1 : diff > 0 ? 1 : 0);
       } else
         return 0;

    case 3:
       diff = art->lines()->numberOfLines() - otherArticle->lines()->numberOfLines();
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    case 4:
       date1 = art->date()->dateTime().toTime_t();
       date2 = otherArticle->date()->dateTime().toTime_t();
       if (art->type() == KNArticle::ATremote && static_cast<KNHeaderView*>( listView() )->sortByThreadChangeDate()) {
         if ( boost::static_pointer_cast<KNRemoteArticle>( art )->subThreadChangeDate() > date1 )
           date1 = boost::static_pointer_cast<KNRemoteArticle>( art )->subThreadChangeDate();
         if ( boost::static_pointer_cast<KNRemoteArticle>( otherArticle )->subThreadChangeDate() > date2 ) {
           date2 = boost::static_pointer_cast<KNRemoteArticle>( otherArticle )->subThreadChangeDate();
         }
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
    pen.setColor( cg.color( QPalette::HighlightedText ) );
    base = cg.color( QPalette::Highlight );
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

    for (int i = 0; i < 4; ++i) {
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
    QString t = p->fontMetrics().elidedText( text( column ), Qt::ElideRight, width - xText - cntWidth - 5  );

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
        pen.setColor( cg.color( QPalette::HighlightedText ) );
      } else {
        pen.setColor( cg.color( QPalette::Link ) );
      }
      p->setPen( pen );
      p->drawText( xText + QFontMetrics( orig ).width( t, -1 ), 0, width - xText - 5, height(), alignment | Qt::AlignVCenter,  t2 );
    }
  }
}


int KNHdrViewItem::width( const QFontMetrics &fm, const Q3ListView *, int column ) const
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
    if ( art->lines()->numberOfLines() != 0 ) { // invalid values are read as '0' in KNGroup::insortHeaders()
      return QString::number( art->lines()->numberOfLines() );
    } else {
      return QString();
    }
  }

  if ( col == hv->paintInfo()->scoreCol ) {
    if ( art->type() == KNArticle::ATremote )
      return QString::number( boost::static_pointer_cast<KNRemoteArticle>( art )->score() );
    else
      return QString();
  }

  if ( col == hv->paintInfo()->dateCol ) {
    return hv->mDateFormatter.dateString( art->date()->dateTime().toTime_t() );
  } else
    return K3ListViewItem::text( col );
}


Q3DragObject* KNHdrViewItem::dragObject()
{
#ifdef __GNUC__
#warning Enable this section again, once KNHdrView does not derive from K3ListView any more and can process QDrag (not Q3DragObject)
#endif
return 0;
#if 0
  QDrag *drag = new QDrag( listView()->viewport() );
  QMimeData *md = new QMimeData;
  drag->setMimeData( md );

  KUrl::List list;
  QString mid = art->messageID()->asUnicodeString();
  // for some obscure reason it returns messageid in <>s
  mid = mid.mid( 1, mid.length() - 2 );
  list.append( QLatin1String( "news:" ) + mid );
  QMap<QString,QString> metadata;
  metadata["labels"] = KUrl::toPercentEncoding( art->subject()->asUnicodeString() );
  list.populateMimeData( md, metadata );
  md->setData( "x-knode-drag/article" , QByteArray() );

  drag->setPixmap( knGlobals.configManager()->appearance()->icon( KNode::Appearance::posting ) );
  return drag;
#endif
}


int KNHdrViewItem::countUnreadInThread()
{
  int count = 0;
  if ( knGlobals.settings()->showUnread() ) {
    if (art->type() == KNArticle::ATremote) {
      count = boost::static_pointer_cast<KNRemoteArticle>( art )->unreadFollowUps();
    }
  }
  return count;
}


bool KNHdrViewItem::greyOut()
{
  if (art->type() == KNArticle::ATremote) {
    return !boost::static_pointer_cast<KNRemoteArticle>( art )->hasUnreadFollowUps()
        && boost::static_pointer_cast<KNRemoteArticle>( art )->isRead();
  } else
    return false;
}


bool KNHdrViewItem::firstColBold()
{
  if(art->type() == KNArticle::ATremote)
    return boost::static_pointer_cast<KNRemoteArticle>( art )->isNew();
  else
    return false;
}


QColor KNHdrViewItem::normalColor()
{
  if (art->type()==KNArticle::ATremote)
    return boost::static_pointer_cast<KNRemoteArticle>( art )->color();
  else
    return knGlobals.settings()->unreadThreadColor();
}


QColor KNHdrViewItem::greyColor()
{
  return knGlobals.settings()->readThreadColor();
}

