/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#include <limits.h>

#include <qcursor.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <kglobalsettings.h>

#include "cardview.h"

#define MIN_ITEM_WIDTH 80

class CardViewTip : public QLabel
{
  public:
    CardViewTip( QWidget *parent = 0, const char *name = 0 )
      : QLabel( parent, name )
    {
      setPalette( QToolTip::palette() );
      setFrameStyle( Panel | Plain );
      setMidLineWidth( 0 );
      setIndent( 1 );
    }

    ~CardViewTip() {};

  protected:
    void leaveEvent( QEvent* )
    {
      hide();
    }
};

//
// Warning: make sure you use findRef() instead of find() to find an
//          item! Only the pointer value is unique in the list.
//
class CardViewItemList : public QPtrList<CardViewItem>
{
  protected:
    virtual int compareItems( QPtrCollection::Item item1,
                              QPtrCollection::Item item2 )
    {
      CardViewItem *cItem1 = (CardViewItem*)item1;
      CardViewItem *cItem2 = (CardViewItem*)item2;

      if ( cItem1 == cItem2 )
          return 0;

      if ( (cItem1 == 0) || (cItem2 == 0) )
          return cItem1 ? -1 : 1;

      if ( cItem1->caption() < cItem2->caption() )
        return -1;
      else if ( cItem1->caption() > cItem2->caption() )
        return 1;

      return 0;
    }
};

class CardViewSeparator
{
  friend class CardView;

  public:
    CardViewSeparator( CardView *view )
      : mView( view )
    {
      mRect = QRect( 0, 0, view->separatorWidth(), 0 );
    }

    ~CardViewSeparator() {}

    void paintSeparator( QPainter *p, QColorGroup &cg )
    {
      p->fillRect( 0, 0, mRect.width(), mRect.height(),
                   cg.brush(QColorGroup::Button) );
    }

    void repaintSeparator()
    {
      mView->repaintContents( mRect );
    }

  private:
    CardView *mView;
    QRect mRect;
};

class CardViewPrivate
{
  public:
    CardViewPrivate()
     : mSelectionMode( CardView::Multi ),
       mDrawCardBorder( true ),
       mDrawFieldLabels( true ),
       mDrawSeparators( true),
       mSepWidth( 2 ),
       mShowEmptyFields( false ),
       mLayoutDirty( true ),
       mLastClickOnItem( false ),
       mItemMargin( 0 ),
       mItemSpacing( 10 ),
       mItemWidth( 200 ),
       mMaxFieldLines( INT_MAX ),
       mCurrentItem( 0L ),
       mLastClickPos( QPoint(0, 0) ),
       mRubberBandAnchor( 0 ),
       mCompText( QString::null )
    {};

    CardViewItemList mItemList;
    QPtrList<CardViewSeparator> mSeparatorList;
    QFontMetrics *mFm;
    QFontMetrics *mBFm;
    QFont mHeaderFont;
    CardView::SelectionMode mSelectionMode;
    bool mDrawCardBorder;
    bool mDrawFieldLabels;
    bool mDrawSeparators;
    int mSepWidth;
    bool mShowEmptyFields;
    bool mLayoutDirty;
    bool mLastClickOnItem;
    uint mItemMargin;           // internal margin in items
    uint mItemSpacing;          // spacing between items, column seperators and border
    int mItemWidth;             // width of all items
    uint mMaxFieldLines;        // Max lines to dispaly pr field
    CardViewItem *mCurrentItem;
    QPoint mLastClickPos;
    QTimer *mTimer;             // times out if mouse rests for more than 500 msecs
    CardViewTip *mTip;          // passed to the item under a resting cursor to display full text
    bool mOnSeparator;          // set/reset on mouse movement
    // for resizing by dragging the separators
    int mResizeAnchor;          // uint, ulong? the mouse down separator left
    int mRubberBandAnchor;      // for erasing rubber bands
    // data used for resizing.
    // as they are beeded by each mouse move while resizing, we store them here,
    // saving 8 calculations in each mouse move.
    int colspace;               // amount of space between items pr column
    uint first;                 // the first col to anchor at for painting rubber bands
    int firstX;                 // X position of first in pixel
    int pressed;                // the colummn that was pressed on at resizing start
    int span;                   // pressed - first
    // key completion
    QString mCompText;          // current completion string
    QDateTime mCompUpdated;     // ...was updated at this time
};

class CardViewItemPrivate
{
  public:
    CardViewItemPrivate() {}

    QString mCaption;
    QPtrList< CardViewItem::Field > mFieldList;
    bool mSelected;
    int x;                      // horizontal position, set by the view
    int y;                      // vertical position, set by the view
    int maxLabelWidth;          // the width of the widest label, according to the view font.
    int hcache;                 // height cache
};


CardViewItem::CardViewItem( CardView *parent, const QString &caption )
  : d( new CardViewItemPrivate() ), mView( parent )
{
  d->mCaption = caption;

  initialize();
}

CardViewItem::~CardViewItem()
{
  // Remove ourself from the view
  if ( mView != 0 )
    mView->takeItem( this );

  delete d;
  d = 0;
}

void CardViewItem::initialize()
{
  d->mSelected = false;
  d->mFieldList.setAutoDelete( true );
  d->maxLabelWidth = 0;
  d->hcache = 0;

  // Add ourself to the view
  if ( mView != 0 )
    mView->insertItem( this );
}

void CardViewItem::paintCard( QPainter *p, QColorGroup &cg )
{
  if ( !mView )
    return;

  QPen pen;
  QBrush brush;
  QFontMetrics fm = *(mView->d->mFm);
  QFontMetrics bFm = *(mView->d->mBFm);
  bool drawLabels = mView->d->mDrawFieldLabels;
  bool drawBorder = mView->d->mDrawCardBorder;
  int mg = mView->itemMargin();
  int w = mView->itemWidth() - (mg * 2);
  int h = height() - (mg * 2);
  const int colonWidth( fm.width( ":" ) );
  int labelXPos = 2 + mg;
  int labelWidth = QMIN( w / 2 - 4 - mg, d->maxLabelWidth + colonWidth + 4 );
  int valueXPos = labelWidth + 4 + mg;
  int valueWidth = w - labelWidth - 4 - mg;

  p->setFont( mView->font() );
  labelWidth -= colonWidth; // extra space for the colon

  if ( !drawLabels ) {
    valueXPos = labelXPos;
    valueWidth = w - 4;
  }

  // Draw a simple box
  if ( isSelected() )
    pen = QPen( cg.highlight(), 1 );
  else
    pen = QPen( cg.button(), 1 );
  p->setPen( pen );

  // Draw the border - this is only draw if the user asks for it.
  if ( drawBorder )
    p->drawRect( mg, mg, w, h );

  // set the proper pen color for the caption box
  if ( isSelected() )
    brush = cg.brush( QColorGroup::Highlight );
  else
    brush = cg.brush( QColorGroup::Button );

  p->fillRect( mg, mg, w, 4 + bFm.height(), brush );

  // Now paint the caption
  p->save();
  QFont bFont = mView->headerFont();
  p->setFont( bFont );
  if ( isSelected() )
    p->setPen( cg.highlightedText() );
  else
    p->setPen( cg.buttonText() );

  p->drawText( 2 + mg, 2 + mg + bFm.ascent(), trimString( d->mCaption, w - 4, bFm ) );
  p->restore();

  // Go through the fields and draw them
  QPtrListIterator<CardViewItem::Field> iter( d->mFieldList );
  QString label, value;
  int yPos = mg + 4 + bFm.height() + fm.height();
  p->setPen( cg.text() );

  int fh = fm.height();
  int cln( 0 );
  QString tmp;
  int maxLines = mView->maxFieldLines();
  for ( iter.toFirst(); iter.current(); ++iter ) {
    value = (*iter)->second;
    if ( value.isEmpty() && ! mView->d->mShowEmptyFields )
      continue;

    if ( drawLabels ) {
      label = trimString( (*iter)->first, labelWidth, fm );
      p->drawText( labelXPos, yPos, label + ":" );
    }

    for ( cln = 0; cln <= maxLines; cln++ ) {
      tmp = value.section( '\n', cln, cln );
      if ( !tmp.isEmpty() )
        p->drawText( valueXPos, yPos + cln * fh, trimString( tmp, valueWidth, fm ) );
      else
        break;
    }

    if ( cln == 0 )
      cln = 1;
    yPos += cln * fh + 2;
  }

  // if we are the current item and the view has focus, draw focus rect
  if ( mView->currentItem() == this && mView->hasFocus() ) {
    mView->style().drawPrimitive( QStyle::PE_FocusRect, p,
        QRect( 0, 0, mView->itemWidth(), h + (2 * mg) ), cg,
        QStyle::Style_FocusAtBorder,
        QStyleOption( isSelected() ? cg.highlight() : cg.base() ) );
  }
}

const QString &CardViewItem::caption() const
{
  return d->mCaption;
}


int CardViewItem::height( bool allowCache ) const
{
  // use cache
  if ( allowCache && d->hcache )
    return d->hcache;

  // Base height:
  //  2 for line width
  //  2 for top caption pad
  //  2 for bottom caption pad
  //   2 pad for the end
  // + 2 times the advised margin
  int baseHeight = 8 + ( 2 * mView->itemMargin() );

  //  size of font for each field
  //  2 pad for each field

  bool sef = mView->showEmptyFields();
  int fh = mView->d->mFm->height();
  int fieldHeight = 0;
  int lines;
  int maxLines( mView->maxFieldLines() );
  QPtrListIterator<CardViewItem::Field> iter( d->mFieldList );
  for ( iter.toFirst(); iter.current(); ++iter ) {
    if ( !sef && (*iter)->second.isEmpty() )
      continue;
    lines = QMIN( (*iter)->second.contains( '\n' ) + 1, maxLines );
    fieldHeight += ( lines * fh ) + 2;
  }

  // height of caption font (bold)
  fieldHeight += mView->d->mBFm->height();
  d->hcache = baseHeight + fieldHeight;
  return d->hcache;
}

bool CardViewItem::isSelected() const
{
  return d->mSelected;
}

void CardViewItem::setSelected( bool selected )
{
  d->mSelected = selected;
}

void CardViewItem::insertField( const QString &label, const QString &value )
{
  CardViewItem::Field *f = new CardViewItem::Field( label, value );
  d->mFieldList.append( f );
  d->hcache = 0;

  if ( mView ) {
    mView->setLayoutDirty( true );
    d->maxLabelWidth = QMAX( mView->d->mFm->width( label ), d->maxLabelWidth );
  }
}

void CardViewItem::removeField( const QString &label )
{
  CardViewItem::Field *f;

  QPtrListIterator<CardViewItem::Field> iter( d->mFieldList );
  for ( iter.toFirst(); iter.current(); ++iter ) {
    f = *iter;
    if ( f->first == label )
      break;
  }

  if (*iter)
    d->mFieldList.remove( *iter );
  d->hcache = 0;

  if ( mView )
    mView->setLayoutDirty( true );
}

void CardViewItem::clearFields()
{
  d->mFieldList.clear();
  d->hcache = 0;

  if ( mView )
    mView->setLayoutDirty( true );
}

QString CardViewItem::trimString( const QString &text, int width,
                                  QFontMetrics &fm ) const
{
  if ( fm.width( text ) <= width )
    return text;

  QString dots = "...";
  int dotWidth = fm.width( dots );
  QString trimmed;
  int charNum = 0;

  while ( fm.width( trimmed ) + dotWidth < width ) {
    trimmed += text[ charNum ];
    charNum++;
  }

  // Now trim the last char, since it put the width over the top
  trimmed = trimmed.left( trimmed.length() - 1 );
  trimmed += dots;

  return trimmed;
}

CardViewItem *CardViewItem::nextItem() const
{
  CardViewItem *item = 0;

  if ( mView )
    item = mView->itemAfter( this );

  return item;
}

void CardViewItem::repaintCard()
{
  if ( mView )
    mView->repaintItem( this );
}

void CardViewItem::setCaption( const QString &caption )
{
  d->mCaption = caption;
  repaintCard();
}

QString CardViewItem::fieldValue( const QString &label ) const
{
  QPtrListIterator<CardViewItem::Field> iter( d->mFieldList );
  for ( iter.toFirst(); iter.current(); ++iter )
    if ( (*iter)->first == label )
        return (*iter)->second;

  return QString();
}


void CardViewItem::showFullString( const QPoint &itempos, CardViewTip *tip )
{
  bool trimmed( false );
  QString s;
  int mrg = mView->itemMargin();
  int y = mView->d->mBFm->height() + 6 + mrg;
  int w = mView->itemWidth() - (2 * mrg);
  int lw;
  bool drawLabels = mView->drawFieldLabels();
  bool isLabel = drawLabels && itempos.x() < w / 2 ? true : false;

  if ( itempos.y() < y ) {
    if ( itempos.y() < 8 + mrg || itempos.y() > y - 4 )
      return;
    // this is the caption
    s = caption();
    trimmed = mView->d->mBFm->width( s ) > w - 4;
    y = 2 + mrg;
    lw = 0;
    isLabel = true;
  } else {
    // find the field
    Field *f = fieldAt( itempos );
    if ( !f || ( !mView->showEmptyFields() && f->second.isEmpty() ) )
      return;

    // y position:
    // header font height + 4px hader margin + 2px leading + item margin
    // + actual field index * (fontheight + 2px leading)
    int maxLines = mView->maxFieldLines();
    bool se = mView->showEmptyFields();
    int fh = mView->d->mFm->height();

    Field *_f;
    for ( _f = d->mFieldList.first(); _f != f; _f = d->mFieldList.next() )
      if ( se || ! _f->second.isEmpty() )
        y += ( QMIN( _f->second.contains( '\n' ) + 1, maxLines ) * fh ) + 2;

    if ( isLabel && itempos.y() > y + fh )
      return;

    s = isLabel ? f->first : f->second;

    int colonWidth = mView->d->mFm->width(":");
    lw = drawLabels ? QMIN( w / 2 - 4 - mrg, d->maxLabelWidth + colonWidth + 4 ) : 0;
    int mw = isLabel ? lw - colonWidth : w - lw - (mrg * 2);
    if ( isLabel ) {
      trimmed = mView->d->mFm->width( s ) > mw - colonWidth;
    } else {
      QRect r( mView->d->mFm->boundingRect( 0, 0, INT_MAX, INT_MAX, Qt::AlignTop|Qt::AlignLeft, s ) );
      trimmed = r.width() > mw || r.height() / fh >  QMIN( s.contains( '\n' ) + 1, maxLines );
    }
  }

  if ( trimmed ) {
    tip->setFont( (isLabel && !lw) ? mView->headerFont() : mView->font() );
    tip->setText( s );
    tip->adjustSize();
    // find a proper position
    int lx;
    lx = isLabel || !drawLabels ? mrg : lw + mrg + 2;
    QPoint pnt( mView->contentsToViewport( QPoint( d->x, d->y ) ) );
    pnt += QPoint( lx, y );
    if ( pnt.x() < 0 )
      pnt.setX( 0 );
    if ( pnt.x() + tip->width() > mView->visibleWidth() )
      pnt.setX( mView->visibleWidth() - tip->width() );
    if ( pnt.y() + tip->height() > mView->visibleHeight() )
      pnt.setY( QMAX( 0, mView->visibleHeight() - tip->height() ) );
    // show
    tip->move( pnt );
    tip->show();
  }
}

CardViewItem::Field *CardViewItem::fieldAt( const QPoint & itempos ) const
{
  int ypos = mView->d->mBFm->height() + 7 + mView->d->mItemMargin;
  int iy = itempos.y();
  // skip below caption
  if ( iy <= ypos )
    return 0;
  // try find a field
  bool showEmpty = mView->showEmptyFields();
  int fh = mView->d->mFm->height();
  int maxLines = mView->maxFieldLines();
  Field *f;
  for ( f = d->mFieldList.first(); f; f = d->mFieldList.next() ) {
    if ( showEmpty || !f->second.isEmpty() )
      ypos += (QMIN( f->second.contains( '\n' )+1, maxLines ) * fh) + 2;
    if ( iy <= ypos )
      break;
  }

  return f ? f : 0;
}


CardView::CardView( QWidget *parent, const char *name )
  : QScrollView( parent, name ),
    d( new CardViewPrivate() )
{
  d->mItemList.setAutoDelete( true );
  d->mSeparatorList.setAutoDelete( true );

  QFont f = font();
  d->mFm = new QFontMetrics( f );
  f.setBold( true );
  d->mHeaderFont = f;
  d->mBFm = new QFontMetrics( f );
  d->mTip = new CardViewTip( viewport() );
  d->mTip->hide();
  d->mTimer = new QTimer( this, "mouseTimer" );

  viewport()->setMouseTracking( true );
  viewport()->setFocusProxy( this );
  viewport()->setFocusPolicy( WheelFocus );
  viewport()->setBackgroundMode( PaletteBase );

  connect( d->mTimer, SIGNAL( timeout() ), this, SLOT( tryShowFullText() ) );

  setBackgroundMode( PaletteBackground, PaletteBase );

  // no reason for a vertical scrollbar
  setVScrollBarMode( AlwaysOff );
}

CardView::~CardView()
{
  delete d->mFm;
  delete d->mBFm;
  delete d;
  d = 0;
}

void CardView::insertItem( CardViewItem *item )
{
  d->mItemList.inSort( item );
  setLayoutDirty( true );
}

void CardView::takeItem( CardViewItem *item )
{
  if ( d->mCurrentItem == item )
    d->mCurrentItem = item->nextItem();
  d->mItemList.take( d->mItemList.findRef( item ) );

  setLayoutDirty( true );
}

void CardView::clear()
{
  d->mItemList.clear();

  setLayoutDirty( true );
}

CardViewItem *CardView::currentItem() const
{
  if ( !d->mCurrentItem && d->mItemList.count() )
    d->mCurrentItem = d->mItemList.first();

  return d->mCurrentItem;
}

void CardView::setCurrentItem( CardViewItem *item )
{
  if ( !item )
    return;
  else if ( item->cardView() != this ) {
    kdDebug(5720)<<"CardView::setCurrentItem: Item ("<<item<<") not owned! Backing out.."<<endl;
    return;
  } else if ( item == currentItem() ) {
    return;
  }

  if ( d->mSelectionMode == Single ) {
    setSelected( item, true );
  } else {
    CardViewItem *it = d->mCurrentItem;
    d->mCurrentItem = item;
    if ( it )
      it->repaintCard();

    item->repaintCard();
  }

  if ( ! d->mOnSeparator )
    ensureItemVisible( item );

  emit currentChanged( item );
}

CardViewItem *CardView::itemAt( const QPoint &viewPos ) const
{
  CardViewItem *item = 0;
  QPtrListIterator<CardViewItem> iter( d->mItemList );
  bool found = false;
  for ( iter.toFirst(); iter.current() && !found; ++iter ) {
    item = *iter;
    if ( QRect( item->d->x, item->d->y, d->mItemWidth, item->height() ).contains( viewPos ) )
      found = true;
  }

  if ( found )
    return item;

  return 0;
}

QRect CardView::itemRect( const CardViewItem *item ) const
{
  return QRect( item->d->x, item->d->y, d->mItemWidth, item->height() );
}

void CardView::ensureItemVisible( const CardViewItem *item )
{
  ensureVisible( item->d->x, item->d->y, d->mItemSpacing, 0 );
  ensureVisible( item->d->x + d->mItemWidth, item->d->y, d->mItemSpacing, 0 );
}

void CardView::repaintItem( const CardViewItem *item )
{
  repaintContents( QRect( item->d->x, item->d->y, d->mItemWidth, item->height() ) );
}

void CardView::setSelectionMode( CardView::SelectionMode mode )
{
  selectAll( false );

  d->mSelectionMode = mode;
}

CardView::SelectionMode CardView::selectionMode() const
{
  return d->mSelectionMode;
}

void CardView::selectAll( bool state )
{
  QPtrListIterator<CardViewItem> iter( d->mItemList );
  if ( !state ) {
    for ( iter.toFirst(); iter.current(); ++iter ) {
      if ( (*iter)->isSelected() ) {
        (*iter)->setSelected( false );
        (*iter)->repaintCard();
      }
    }

    emit selectionChanged( 0 );
  } else if ( d->mSelectionMode != CardView::Single ) {
    for ( iter.toFirst(); iter.current(); ++iter ) {
      (*iter)->setSelected( true );
    }

    if ( d->mItemList.count() > 0 ) {
      // emit, since there must have been at least one selected
      emit selectionChanged();
      viewport()->update();
    }
  }
}

void CardView::setSelected( CardViewItem *item, bool selected )
{
  if ( (item == 0) || (item->isSelected() == selected) )
    return;

  if ( selected && d->mCurrentItem != item ) {
    CardViewItem *it = d->mCurrentItem;
    d->mCurrentItem = item;
    if ( it )
      it->repaintCard();
  }

  if ( d->mSelectionMode == CardView::Single ) {
    bool b = signalsBlocked();
    blockSignals( true );
    selectAll( false );
    blockSignals( b );

    if ( selected ) {
      item->setSelected( selected );
      item->repaintCard();
      emit selectionChanged();
      emit selectionChanged( item );
    } else {
      emit selectionChanged();
      emit selectionChanged( 0 );
    }
  } else if ( d->mSelectionMode == CardView::Multi ) {
    item->setSelected( selected );
    item->repaintCard();
    emit selectionChanged();
  } else if ( d->mSelectionMode == CardView::Extended ) {
    bool b = signalsBlocked();
    blockSignals( true );
    selectAll( false );
    blockSignals( b );

    item->setSelected( selected );
    item->repaintCard();
    emit selectionChanged();
  }
}

bool CardView::isSelected( CardViewItem *item ) const
{
  return (item && item->isSelected());
}

CardViewItem *CardView::selectedItem() const
{
  // find the first selected item
  QPtrListIterator<CardViewItem> iter( d->mItemList );
  for ( iter.toFirst(); iter.current(); ++iter ) {
    if ( (*iter)->isSelected() )
      return *iter;
  }

  return 0;
}

CardViewItem *CardView::firstItem() const
{
  return d->mItemList.first();
}

int CardView::childCount() const
{
  return d->mItemList.count();
}

CardViewItem *CardView::findItem( const QString &text, const QString &label,
                                  Qt::StringComparisonMode compare ) const
{
  // If the text is empty, we will return null, since empty text will
  // match anything!
  if ( text.isEmpty() )
    return 0;

  QPtrListIterator<CardViewItem> iter( d->mItemList );
  if ( compare & Qt::BeginsWith ) {
    QString value;
    for ( iter.toFirst(); iter.current(); ++iter ) {
      value = (*iter)->fieldValue( label ).upper();
      if ( value.startsWith( text.upper() ) )
        return *iter;
    }
  } else {
    kdDebug(5720) << "CardView::findItem: search method not implemented" << endl;
  }

  return 0;
}

uint CardView::columnWidth() const
{
  return d->mDrawSeparators ?
    d->mItemWidth + ( 2 * d->mItemSpacing ) + d->mSepWidth :
    d->mItemWidth + d->mItemSpacing;
}

void CardView::drawContents( QPainter *p, int clipx, int clipy,
                             int clipw, int cliph )
{
  QScrollView::drawContents( p, clipx, clipy, clipw, cliph );

 if ( d->mLayoutDirty )
   calcLayout();

  // allow setting costum colors in the viewport pale
  QColorGroup cg = viewport()->palette().active();

  QRect clipRect( clipx, clipy, clipw, cliph );
  QRect cardRect;
  QRect sepRect;
  CardViewItem *item;
  CardViewSeparator *sep;

  // make sure the viewport is a pure background
  viewport()->erase( clipRect );

  // Now tell the cards to draw, if they are in the clip region
  QPtrListIterator<CardViewItem> iter( d->mItemList );
  for ( iter.toFirst(); iter.current(); ++iter) {
    item = *iter;
    cardRect.setRect( item->d->x, item->d->y, d->mItemWidth, item->height() );

    if ( clipRect.intersects( cardRect ) || clipRect.contains( cardRect ) ) {
      // Tell the card to paint
      p->save();
      p->translate( cardRect.x(), cardRect.y() );
      item->paintCard( p, cg );
      p->restore();
    }
  }

  // Followed by the separators if they are in the clip region
  QPtrListIterator<CardViewSeparator> sepIter( d->mSeparatorList );
  for ( sepIter.toFirst(); sepIter.current(); ++sepIter ) {
    sep = *sepIter;
    sepRect = sep->mRect;

    if ( clipRect.intersects( sepRect ) || clipRect.contains( sepRect ) ) {
      p->save();
      p->translate( sepRect.x(), sepRect.y() );
      sep->paintSeparator( p, cg );
      p->restore();
    }
  }
}

void CardView::resizeEvent( QResizeEvent *event )
{
  QScrollView::resizeEvent( event );

  setLayoutDirty( true );
}

void CardView::calcLayout()
{
  // Start in the upper left corner and layout all the
  // cars using their height and width
  int maxWidth = 0;
  int maxHeight = 0;
  int xPos = 0;
  int yPos = 0;
  int cardSpacing = d->mItemSpacing;

  // delete the old separators
  d->mSeparatorList.clear();

  QPtrListIterator<CardViewItem> iter( d->mItemList );
  CardViewItem *item = 0;
  CardViewSeparator *sep = 0;
  xPos += cardSpacing;

  for ( iter.toFirst(); iter.current(); ++iter ) {
    item = *iter;

    yPos += cardSpacing;

    if ( yPos + item->height() + cardSpacing >= height() - horizontalScrollBar()->height() ) {
      maxHeight = QMAX( maxHeight, yPos );

      // Drawing in this column would be greater than the height
      // of the scroll view, so move to next column
      yPos = cardSpacing;
      xPos += cardSpacing + maxWidth;
      if ( d->mDrawSeparators ) {
        // Create a separator since the user asked
        sep = new CardViewSeparator( this );
        sep->mRect.moveTopLeft( QPoint( xPos, yPos + d->mItemMargin ) );
        xPos += d->mSepWidth + cardSpacing;
        d->mSeparatorList.append( sep );
      }

      maxWidth = 0;
    }

    item->d->x = xPos;
    item->d->y = yPos;

    yPos += item->height();
    maxWidth = QMAX( maxWidth, d->mItemWidth );
  }

  xPos += maxWidth;
  resizeContents( xPos + cardSpacing, maxHeight );

  // Update the height of all the separators now that we know the
  // max height of a column
  QPtrListIterator<CardViewSeparator> sepIter( d->mSeparatorList );
  for ( sepIter.toFirst(); sepIter.current(); ++sepIter )
    (*sepIter)->mRect.setHeight( maxHeight - 2 * cardSpacing - 2 * d->mItemMargin );

  d->mLayoutDirty = false;
}

CardViewItem *CardView::itemAfter( const CardViewItem *item ) const
{
  d->mItemList.findRef( item );
  return d->mItemList.next();
}

uint CardView::itemMargin() const
{
  return d->mItemMargin;
}

void CardView::setItemMargin( uint margin )
{
  if ( margin == d->mItemMargin )
    return;

  d->mItemMargin = margin;
  setLayoutDirty( true );
}

uint CardView::itemSpacing() const
{
  return d->mItemSpacing;
}

void CardView::setItemSpacing( uint spacing )
{
  if ( spacing == d->mItemSpacing )
    return;

  d->mItemSpacing = spacing;
  setLayoutDirty( true );
}

void CardView::contentsMousePressEvent( QMouseEvent *e )
{
  QScrollView::contentsMousePressEvent( e );

  QPoint pos = e->pos();
  d->mLastClickPos = pos;

  CardViewItem *item = itemAt( pos );

  if ( item == 0 ) {
    d->mLastClickOnItem = false;
    if ( d->mOnSeparator) {
      d->mResizeAnchor = e->x() + contentsX();
      d->colspace = (2 * d->mItemSpacing);
      int ccw = d->mItemWidth + d->colspace + d->mSepWidth;
      d->first = (contentsX() + d->mSepWidth) / ccw;
      d->pressed = (d->mResizeAnchor + d->mSepWidth) / ccw;
      d->span = d->pressed - d->first;
      d->firstX = d->first * ccw;
      if ( d->firstX )
        d->firstX -= d->mSepWidth;
    } else {
      selectAll( false );
    }

    return;
  }

  d->mLastClickOnItem = true;

  CardViewItem *other = d->mCurrentItem;
  setCurrentItem( item );

  // Always emit the selection
  emit clicked( item );

  // The RMB click
  if ( e->button() & Qt::RightButton ) {
    // clear previous selection
    bool blocked = signalsBlocked();
    blockSignals( true );
    selectAll( false );
    blockSignals( blocked );

    // select current item
    item->setSelected( true );

    emit contextMenuRequested( item, mapToGlobal( e->pos() ) );
    return;
  }

  // Check the selection type and update accordingly
  if ( d->mSelectionMode == CardView::Single ) {
    // make sure it isn't already selected
    if ( item->isSelected() )
      return;

    bool b = signalsBlocked();
    blockSignals( true );
    selectAll( false );
    blockSignals( b );

    item->setSelected( true );
    item->repaintCard();
    emit selectionChanged( item );
  } else if ( d->mSelectionMode == CardView::Multi ) {
    // toggle the selection
    item->setSelected( !item->isSelected() );
    item->repaintCard();
    emit selectionChanged();
  } else if ( d->mSelectionMode == CardView::Extended ) {
    if ( (e->button() & Qt::LeftButton) && (e->state() & Qt::ShiftButton) ) {
      if ( item == other )
        return;

      bool s = !item->isSelected();

      if ( s && !(e->state() & ControlButton) ) {
        bool b = signalsBlocked();
        blockSignals( true );
        selectAll( false );
        blockSignals( b );
      }

      int from, to, a, b;
      a = d->mItemList.findRef( item );
      b = d->mItemList.findRef( other );
      from = a < b ? a : b;
      to = a > b ? a : b;

      CardViewItem *aItem;
      for ( ; from <= to; from++ ) {
        aItem = d->mItemList.at( from );
        aItem->setSelected( s );
        repaintItem( aItem );
      }

      emit selectionChanged();
    } else if ( (e->button() & Qt::LeftButton) && (e->state() & Qt::ControlButton) ) {
      item->setSelected( !item->isSelected() );
      item->repaintCard();
      emit selectionChanged();
    } else if ( e->button() & Qt::LeftButton ) {
      bool b = signalsBlocked();
      blockSignals( true );
      selectAll( false );
      blockSignals( b );

      item->setSelected( true );
      item->repaintCard();
      emit selectionChanged();
    }
  }
}

void CardView::contentsMouseReleaseEvent( QMouseEvent *e )
{
  QScrollView::contentsMouseReleaseEvent( e );

  if ( d->mResizeAnchor && d->span ) {
    unsetCursor();
    // hide rubber bands
    int newiw = d->mItemWidth - ((d->mResizeAnchor - d->mRubberBandAnchor) / d->span);
    drawRubberBands( 0 );
    // we should move to reflect the new position if we are scrolled.
    if ( contentsX() ) {
      int newX = QMAX( 0, ( d->pressed * ( newiw + d->colspace + d->mSepWidth ) ) - e->x() );
      setContentsPos( newX, contentsY() );
    }
    // set new item width
    setItemWidth( newiw );
    // reset anchors
    d->mResizeAnchor = 0;
    d->mRubberBandAnchor = 0;
    return;
  }

  // If there are accel keys, we will not emit signals
  if ( (e->state() & Qt::ShiftButton) || (e->state() & Qt::ControlButton) )
    return;

  // Get the item at this position
  CardViewItem *item = itemAt( e->pos() );

  if ( item && KGlobalSettings::singleClick() )
    emit executed( item );
}

void CardView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  QScrollView::contentsMouseDoubleClickEvent( e );

  CardViewItem *item = itemAt( e->pos() );

  if ( item )
    d->mCurrentItem = item;

  if ( item && !KGlobalSettings::singleClick() )
    emit executed(item);

  emit doubleClicked( item );
}

void CardView::contentsMouseMoveEvent( QMouseEvent *e )
{
  // resizing
  if ( d->mResizeAnchor ) {
    int x = e->x();
    if ( x != d->mRubberBandAnchor )
      drawRubberBands( x );
      return;
  }

  if ( d->mLastClickOnItem && (e->state() & Qt::LeftButton) &&
       ((e->pos() - d->mLastClickPos).manhattanLength() > 4)) {

    startDrag();
    return;
  }

  d->mTimer->start( 500 );

  // see if we are over a separator
  // only if we actually have them painted?
  if ( d->mDrawSeparators  ) {
    int colcontentw = d->mItemWidth + (2 * d->mItemSpacing);
    int colw = colcontentw + d->mSepWidth;
    int m = e->x() % colw;
    if ( m >= colcontentw && m > 0 ) {
      setCursor( SplitHCursor );
      d->mOnSeparator = true;
    } else {
      setCursor( ArrowCursor );
      d->mOnSeparator = false;
    }
  }
}

void CardView::enterEvent( QEvent* )
{
  d->mTimer->start( 500 );
}

void CardView::leaveEvent( QEvent* )
{
  d->mTimer->stop();
  if ( d->mOnSeparator ) {
    d->mOnSeparator = false;
    setCursor( ArrowCursor );
  }
}

void CardView::focusInEvent( QFocusEvent* )
{
  if ( !d->mCurrentItem && d->mItemList.count() )
    setCurrentItem( d->mItemList.first() );
  else if ( d->mCurrentItem )
    d->mCurrentItem->repaintCard();
}

void CardView::focusOutEvent( QFocusEvent* )
{
  if ( d->mCurrentItem )
    d->mCurrentItem->repaintCard();
}

void CardView::keyPressEvent( QKeyEvent *e )
{
  if ( !(childCount() && d->mCurrentItem) ) {
    e->ignore();
    return;
  }

  uint pos = d->mItemList.findRef( d->mCurrentItem );
  CardViewItem *aItem = 0;
  CardViewItem *old = d->mCurrentItem;

  switch ( e->key() ) {
    case Key_Up:
      if ( pos > 0 ) {
        aItem = d->mItemList.at( pos - 1 );
        setCurrentItem( aItem );
      }
      break;
    case Key_Down:
      if ( pos < d->mItemList.count() - 1 ) {
        aItem = d->mItemList.at( pos + 1 );
        setCurrentItem( aItem );
      }
      break;
    case Key_Left:
    {
      // look for an item in the previous/next column, starting from
      // the vertical middle of the current item.
      // FIXME use nice calculatd measures!!!
      QPoint aPoint( d->mCurrentItem->d->x, d->mCurrentItem->d->y );
      aPoint -= QPoint( 30, -(d->mCurrentItem->height() / 2) );
      aItem = itemAt( aPoint );
      // maybe we hit some space below an item
      while ( !aItem && aPoint.y() > 27 ) {
        aPoint -= QPoint( 0, 16 );
        aItem = itemAt( aPoint );
      }
      if ( aItem )
        setCurrentItem( aItem );

      break;
    }
    case Key_Right:
    {
      // FIXME use nice calculated measures!!!
      QPoint aPoint( d->mCurrentItem->d->x + d->mItemWidth, d->mCurrentItem->d->y );
      aPoint += QPoint( 30, (d->mCurrentItem->height() / 2) );
      aItem = itemAt( aPoint );
      while ( !aItem && aPoint.y() > 27 ) {
        aPoint -= QPoint( 0, 16 );
        aItem = itemAt( aPoint );
      }
      if ( aItem )
        setCurrentItem( aItem );

      break;
    }
    case Key_Home:
      aItem = d->mItemList.first();
      setCurrentItem( aItem );
      break;
    case Key_End:
      aItem = d->mItemList.last();
      setCurrentItem( aItem );
      break;
    case Key_Prior: // PageUp
    {
      // QListView: "Make the item above the top visible and current"
      // TODO if contentsY(), pick the top item of the leftmost visible column
      if ( contentsX() <= 0 )
        return;
      int cw = columnWidth();
      int theCol = ( QMAX( 0, ( contentsX() / cw) * cw ) ) + d->mItemSpacing;
      aItem = itemAt( QPoint( theCol + 1, d->mItemSpacing + 1 ) );
      if ( aItem )
        setCurrentItem( aItem );

      break;
    }
    case Key_Next:  // PageDown
    {
      // QListView: "Make the item below the bottom visible and current"
      // find the first not fully visible column.
      // TODO: consider if a partly visible (or even hidden) item at the
      //       bottom of the rightmost column exists
      int cw = columnWidth();
      int theCol = ( (( contentsX() + visibleWidth() ) / cw) * cw ) + d->mItemSpacing + 1;
      // if separators are on, we may need to we may be one column further right if only the spacing/sep is hidden
      if ( d->mDrawSeparators && cw - (( contentsX() + visibleWidth() ) % cw) <= int( d->mItemSpacing + d->mSepWidth ) )
        theCol += cw;

      // make sure this is not too far right
      while ( theCol > contentsWidth() )
        theCol -= columnWidth();

      aItem = itemAt( QPoint( theCol, d->mItemSpacing + 1 ) );

      if ( aItem )
        setCurrentItem( aItem );

      break;
    }
    case Key_Space:
      setSelected( d->mCurrentItem, !d->mCurrentItem->isSelected() );
      emit selectionChanged();
      break;
    case Key_Return:
    case Key_Enter:
      emit returnPressed( d->mCurrentItem );
      emit executed( d->mCurrentItem );
      break;
    case Key_Menu:
      emit contextMenuRequested( d->mCurrentItem, viewport()->mapToGlobal( 
                                 itemRect(d->mCurrentItem).center() ) );
      break;
    default:
      if ( (e->state() & ControlButton) && e->key() == Key_A ) {
        // select all
        selectAll( true );
        break;
      } else if ( !e->text().isEmpty() && e->text()[ 0 ].isPrint() ) {
        // if we have a string, do autosearch
      }
      break;
  }

  // handle selection
  if ( aItem ) {
    if ( d->mSelectionMode == CardView::Extended ) {
      if ( e->state() & ShiftButton ) {
        // shift button: toggle range
        // if control button is pressed, leave all items
        // and toggle selection current->old current
        // otherwise, ??????
        bool s = ! aItem->isSelected();
        int from, to, a, b;
        a = d->mItemList.findRef( aItem );
        b = d->mItemList.findRef( old );
        from = a < b ? a : b;
        to = a > b ? a : b;

        if ( to - from > 1 ) {
          bool b = signalsBlocked();
          blockSignals( true );
          selectAll( false );
          blockSignals( b );
        }

        CardViewItem *item;
        for ( ; from <= to; from++ ) {
          item = d->mItemList.at( from );
          item->setSelected( s );
          repaintItem( item );
        }

        emit selectionChanged();
      } else if ( e->state() & ControlButton ) {
        // control button: do nothing
      } else {
        // no button: move selection to this item
        bool b = signalsBlocked();
        blockSignals( true );
        selectAll( false );
        blockSignals( b );

        setSelected( aItem, true );
        emit selectionChanged();
      }
    }
  }
}

void CardView::contentsWheelEvent( QWheelEvent *e )
{
  scrollBy( 2 * e->delta() / -3, 0 );
}

void CardView::setLayoutDirty( bool dirty )
{
  if ( d->mLayoutDirty != dirty ) {
    d->mLayoutDirty = dirty;
    repaint();
  }
}

void CardView::setDrawCardBorder( bool enabled )
{
  if ( enabled != d->mDrawCardBorder ) {
    d->mDrawCardBorder = enabled;
    repaint();
  }
}

bool CardView::drawCardBorder() const
{
  return d->mDrawCardBorder;
}

void CardView::setDrawColSeparators( bool enabled )
{
  if ( enabled != d->mDrawSeparators ) {
    d->mDrawSeparators = enabled;
    setLayoutDirty( true );
  }
}

bool CardView::drawColSeparators() const
{
  return d->mDrawSeparators;
}

void CardView::setDrawFieldLabels( bool enabled )
{
  if ( enabled != d->mDrawFieldLabels ) {
    d->mDrawFieldLabels = enabled;
    repaint();
  }
}

bool CardView::drawFieldLabels() const
{
  return d->mDrawFieldLabels;
}

void CardView::setShowEmptyFields( bool show )
{
  if ( show != d->mShowEmptyFields ) {
    d->mShowEmptyFields = show;
    setLayoutDirty( true );
  }
}

bool CardView::showEmptyFields() const
{
  return d->mShowEmptyFields;
}

void CardView::startDrag()
{
  // The default implementation is a no-op. It must be
  // reimplemented in a subclass to be useful
}

void CardView::tryShowFullText()
{
  d->mTimer->stop();
  // if we have an item
  QPoint cpos = viewportToContents( viewport()->mapFromGlobal( QCursor::pos() ) );
  CardViewItem *item = itemAt( cpos );
  if ( item ) {
    // query it for a value to display
    QPoint ipos = cpos - itemRect( item ).topLeft();
    item->showFullString( ipos, d->mTip );
  }
}

void CardView::drawRubberBands( int pos )
{
  if ( pos && d &&
       (!d->span || ((pos - d->firstX) / d->span) - d->colspace - d->mSepWidth < MIN_ITEM_WIDTH) )
    return;

  int tmpcw = (d->mRubberBandAnchor - d->firstX) / d->span;
  int x = d->firstX + tmpcw - d->mSepWidth - contentsX();
  int h = visibleHeight();

  QPainter p( viewport() );
  p.setRasterOp( XorROP );
  p.setPen( gray );
  p.setBrush( gray );
  uint n = d->first;
  // erase
  if ( d->mRubberBandAnchor )
    do {
      p.drawRect( x, 0, 2, h );
      x += tmpcw;
      n++;
    } while ( x < visibleWidth() && n < d->mSeparatorList.count() );
  // paint new
  if ( ! pos )
    return;
  tmpcw = (pos - d->firstX) / d->span;
  n = d->first;
  x = d->firstX + tmpcw - d->mSepWidth - contentsX();
  do {
      p.drawRect( x, 0, 2, h );
      x += tmpcw;
      n++;
  } while ( x < visibleWidth() && n < d->mSeparatorList.count() );
  d->mRubberBandAnchor = pos;
}

int CardView::itemWidth() const
{
  return d->mItemWidth;
}

void CardView::setItemWidth( int w )
{
  if ( w == d->mItemWidth )
    return;
  if ( w < MIN_ITEM_WIDTH )
    w = MIN_ITEM_WIDTH;
  d->mItemWidth = w;
  setLayoutDirty( true );
  updateContents();
}

void CardView::setHeaderFont( const QFont &fnt )
{
  d->mHeaderFont = fnt;
  delete d->mBFm;
  d->mBFm = new QFontMetrics( fnt );
}

QFont CardView::headerFont() const
{
  return d->mHeaderFont;
}

void CardView::setFont( const QFont &fnt )
{
  QScrollView::setFont( fnt );
  delete d->mFm;
  d->mFm = new QFontMetrics( fnt );
}

int CardView::separatorWidth() const
{
  return d->mSepWidth;
}

void CardView::setSeparatorWidth( int width )
{
  d->mSepWidth = width;
  setLayoutDirty( true );
}

int CardView::maxFieldLines() const
{
  return d->mMaxFieldLines;
}

void CardView::setMaxFieldLines( int howmany )
{
  d->mMaxFieldLines = howmany ? howmany : INT_MAX;
  // FIXME update, forcing the items to recalc height!!
}

#include "cardview.moc"
