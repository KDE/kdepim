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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "cardview.h"

#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kglobalsettings.h>

//////////////////////////////////////
// CardViewItemList


//
// Warning: make sure you use findRef() instead of find() to find an
//          item! Only the pointer value is unique in the list.
//
class CardViewItemList : public QPtrList<CardViewItem>
{
  protected:
    virtual int compareItems(QPtrCollection::Item item1,
                             QPtrCollection::Item item2)
    {
      CardViewItem *cItem1 = (CardViewItem*)item1;
      CardViewItem *cItem2 = (CardViewItem*)item2;

      if ( cItem1 == cItem2 )
          return 0;
      
      if ((cItem1 == 0) || (cItem2 == 0))
          return cItem1 ? -1 : 1;

      if (cItem1->caption() < cItem2->caption())
        return -1;

      else if (cItem1->caption() > cItem2->caption())
        return 1;

      return 0;
    }

  private:
    int find( const CardViewItem * ) { qDebug("DON'T USE CardViewItemList::find( item )! Use findRef( item )!"); }
};

//////////////////////////////////////
// CardViewSeparator
class CardViewSeparator
{
  friend class CardView;

  public:
    CardViewSeparator(CardView *view)
      : mView(view)
    {
      mRect = QRect(0, 0, 2, 0);
    }

    ~CardViewSeparator() {}

    void paintSeparator(QPainter *p, QColorGroup &cg)
    {
      p->fillRect(0, 0, mRect.width(), mRect.height(),
                  cg.brush(QColorGroup::Button));
    }

    void repaintSeparator()
    {
      mView->repaintContents(mRect);
    }

  private:
    CardView *mView;
    QRect mRect;
};

//////////////////////////////////////
// Private Data

class CardViewPrivate
{
  public:
    CardViewPrivate() {}

    CardViewItemList mItemList;
    QPtrList<CardViewSeparator> mSeparatorList;
    QFontMetrics *mFm;
    QFontMetrics *mBFm;   // bold font
    CardView::SelectionMode mSelectionMode;
    bool mDrawCardBorder;
    bool mDrawSeparators;
    bool mDrawFieldLabels;
    bool mLayoutDirty;
    bool mLastClickOnItem;
    QPoint mLastClickPos;
};

class CardViewItemPrivate
{
  public:
    CardViewItemPrivate() {}

    QString mCaption;
    QPtrList< CardViewItem::Field > mFieldList;
    bool mSelected;
    QRect mRect;
    int mWidth;
};

//////////////////////////////////////
// CardViewItem

CardViewItem::CardViewItem(CardView *parent, QString caption)
  : d(new CardViewItemPrivate()), mView(parent)
{
  d->mCaption = caption;

  initialize();
}

CardViewItem::~CardViewItem()
{
  // Remove ourself from the view
  if (mView != 0)
    mView->takeItem(this);

  delete d;
}

void CardViewItem::initialize()
{
  d->mSelected = false;
  d->mRect = QRect(0, 0, 0, 0);
  d->mWidth = 200;
  d->mFieldList.setAutoDelete(true);

  calcRect();

 // Add ourself to the view
  if (mView != 0)
    mView->insertItem(this);
}

void CardViewItem::paintCard(QPainter *p, QColorGroup &cg)
{

  if (!mView)
    return;

  QPen pen;
  QBrush brush;
  QFontMetrics fm = *(mView->d->mFm);
  QFontMetrics bFm = *(mView->d->mBFm);
  bool drawLabels = mView->d->mDrawFieldLabels;
  bool drawBorder = mView->d->mDrawCardBorder;
  int labelXPos = 2;
  int labelWidth = d->mWidth/2 - 4;
  int valueXPos = d->mWidth/2;
  int valueWidth = d->mWidth/2 - 4;

  if (!drawLabels)
  {
    valueXPos = labelXPos;
    valueWidth = d->mWidth - 4;
  }

  // Draw a simple box
  if (isSelected())
    pen = QPen(cg.highlight(), 1);
  else
    pen = QPen(cg.button(), 1);
  p->setPen(pen);

  // Draw the border - this is only draw if the user asks for it.
  if (drawBorder)
    p->drawRect(0, 0, d->mRect.width(), d->mRect.height());

  //kdDebug() << "CardViewItem::paintCard:" << d->mRect.width() << ", "
  //          << d->mRect.height() << ", " << bFm.height() << endl;


  // set the proper pen color for the caption box
  if (isSelected())
    brush = cg.brush(QColorGroup::Highlight);
  else
    brush = cg.brush(QColorGroup::Button);
  p->fillRect(0, 0, d->mRect.width(), 4 + bFm.height(), brush);

  // Now paint the caption
  p->save();
  QFont bFont = p->font();
  bFont.setBold(true);
  p->setFont(bFont);
  if (isSelected())
    p->setPen(cg.highlightedText());
  else
    p->setPen(cg.text());
  p->drawText(2, 1 + bFm.height(), trimString(d->mCaption, d->mWidth-4, bFm));
  p->restore();

  // Go through the fields and draw them
  QPtrListIterator< CardViewItem::Field > iter(d->mFieldList);
  QString label, value;
  int yPos = 4 + bFm.height() + 1 + fm.height();
  p->setPen(cg.text());
  for (iter.toFirst(); iter.current(); ++iter)
  {
    label = trimString((*iter)->first, labelWidth, fm);
    value = trimString((*iter)->second, valueWidth, fm);

    if (drawLabels)
    {
      p->drawText(labelXPos, yPos, label);
      p->drawText(labelXPos + fm.width(label), yPos, ":");
    }
    p->drawText(valueXPos, yPos, value);

    yPos += fm.height() + 2;
  }
}

const QString &CardViewItem::caption() const
{
  return d->mCaption;
}

void CardViewItem::calcRect()
{
  // Only update the w, h. The view will handle always
  // updating the x, y

  // Base height:
  //  2 for line width
  //  2 for top caption pad
  //  2 for bottom caption pad
  //  2 pad for the end
  int baseHeight = 8;

  //  size of font for each field
  //  2 pad for each field
  int fieldHeight = d->mFieldList.count() * (mView->d->mFm->height() + 2);

  // height of caption font (bold)
  fieldHeight += mView->d->mBFm->height();

  // Width is hard coded

  //kdDebug() << "CardViewItem::calcRect: " << endl;

  d->mRect.setHeight(fieldHeight + baseHeight);
  d->mRect.setWidth(d->mWidth);
}

bool CardViewItem::isSelected() const
{
  return d->mSelected;
}

void CardViewItem::setSelected(bool selected)
{
  d->mSelected = selected;
}

void CardViewItem::insertField(const QString &label, const QString &value)
{
  CardViewItem::Field *f = new CardViewItem::Field(label, value);
  d->mFieldList.append(f);
  calcRect();

  if (mView)
    mView->setLayoutDirty(true);
}

void CardViewItem::removeField(const QString &label)
{
  CardViewItem::Field *f;

  QPtrListIterator< CardViewItem::Field > iter(d->mFieldList);
  for (iter.toFirst(); iter.current(); ++iter)
  {
    f = *iter;
    if (f->first == label)
      break;
  }

  if (*iter)
    d->mFieldList.remove(*iter);

  calcRect();

  if (mView)
    mView->setLayoutDirty(true);
}

void CardViewItem::clearFields()
{
  d->mFieldList.clear();
  calcRect();

  if (mView)
    mView->setLayoutDirty(true);
}

QString CardViewItem::trimString(const QString &text, int width,
                                 QFontMetrics &fm)
{
  if (fm.width(text) <= width)
    return text;

  QString dots = "...";
  int dotWidth = fm.width(dots);
  QString trimmed;
  int charNum = 0;

  while (fm.width(trimmed) + dotWidth < width)
  {
    trimmed += text[charNum];
    charNum++;
  }

  // Now trim the last char, since it put the width over the top
  trimmed = trimmed.left(trimmed.length()-1);
  trimmed += dots;

  return trimmed;
}

CardViewItem *CardViewItem::nextItem()
{
  CardViewItem *item = 0;

  if (mView)
    item = mView->itemAfter(this);

  return item;
}

void CardViewItem::repaintCard()
{
  if (mView)
    mView->viewport()->repaint();
}

void CardViewItem::setCaption(const QString &caption)
{
  d->mCaption = caption;

  if (mView)
    mView->viewport()->repaint();
}

QString CardViewItem::fieldValue(const QString &label)
{
  QPtrListIterator< CardViewItem::Field > iter(d->mFieldList);
  for (iter.toFirst(); iter.current(); ++iter)
    if ((*iter)->first == label)
        return (*iter)->second;

  return QString();
}

//////////////////////////////////////
// CardView

CardView::CardView(QWidget *parent, const char *name)
  : QScrollView(parent, name), d(new CardViewPrivate())
{
  d->mItemList.setAutoDelete(true);
  d->mSeparatorList.setAutoDelete(true);

  QFont f = font();
  d->mFm = new QFontMetrics(f);
  f.setBold(true);
  d->mBFm = new QFontMetrics(f);
  d->mSelectionMode = CardView::Multi;
  d->mDrawCardBorder = true;
  d->mDrawFieldLabels = true;
  d->mDrawSeparators = true;
  d->mLayoutDirty = true;
  d->mLastClickOnItem = false;
  d->mLastClickPos = QPoint(0, 0);

  viewport()->setFocusProxy(this);
  viewport()->setFocusPolicy(WheelFocus);
  viewport()->setBackgroundMode(NoBackground);
  setBackgroundMode(PaletteBackground, PaletteBase);
}

CardView::~CardView()
{
  delete d;
}

void CardView::insertItem(CardViewItem *item)
{
  d->mItemList.inSort(item);

  setLayoutDirty(true);
}

void CardView::takeItem(CardViewItem *item)
{
  d->mItemList.take(d->mItemList.findRef(item));

  setLayoutDirty(true);
}

void CardView::clear()
{
  d->mItemList.clear();

  setLayoutDirty(true);
}

CardViewItem *CardView::itemAt(const QPoint &viewPos)
{
  CardViewItem *item = 0;
  QPtrListIterator<CardViewItem> iter(d->mItemList);
  bool found = false;
  for (iter.toFirst(); iter.current() && !found; ++iter)
  {
    item = *iter;
    if (item->d->mRect.contains(viewPos))
      found = true;
  }

  if (found)
    return item;

  return 0;
}

QRect CardView::itemRect(const CardViewItem *item)
{
  return item->d->mRect;
}

void CardView::ensureItemVisible(const CardViewItem *item)
{
  QRect cardRect = item->d->mRect;

  ensureVisible(cardRect.x(), cardRect.y(),
                cardRect.width(), cardRect.height());
}

void CardView::setSelectionMode(CardView::SelectionMode mode)
{
  selectAll(false);

  d->mSelectionMode = mode;
}

CardView::SelectionMode CardView::selectionMode() const
{
  return d->mSelectionMode;
}

void CardView::selectAll(bool state)
{
  QPtrListIterator<CardViewItem> iter(d->mItemList);
  if (!state)
  {
    for (iter.toFirst(); iter.current(); ++iter)
    {
      if ((*iter)->isSelected())
      {
        (*iter)->setSelected(false);
        (*iter)->repaintCard();
      }
    }

    emit selectionChanged();
    emit selectionChanged(0);
  }
  else if (d->mSelectionMode != CardView::Single)
  {
    for (iter.toFirst(); iter.current(); ++iter)
    {
      (*iter)->setSelected(true);
    }

    if (d->mItemList.count() > 0)
    {
      // emit, since there must have been at least one selected
      emit selectionChanged();
      repaint();
    }
  }
}

void CardView::setSelected(CardViewItem *item, bool selected)
{
  if ((item == 0) || (item->isSelected() == selected))
    return;

  if (d->mSelectionMode == CardView::Single)
  {
    bool b = signalsBlocked();
    blockSignals(true);
    selectAll(false);
    blockSignals(b);

    if (selected)
    {
      item->setSelected(selected);
      item->repaintCard();
      emit selectionChanged();
      emit selectionChanged(item);
    }
    else
    {
      emit selectionChanged();
      emit selectionChanged(0);
    }
  }
  else if (d->mSelectionMode == CardView::Multi)
  {
    item->setSelected(selected);
    item->repaintCard();
    emit selectionChanged();
  }
  else if (d->mSelectionMode == CardView::Extended)
  {
    bool b = signalsBlocked();
    blockSignals(true);
    selectAll(false);
    blockSignals(b);

    item->setSelected(selected);
    item->repaintCard();
    emit selectionChanged();
  }
}

bool CardView::isSelected(CardViewItem *item) const
{
  return (item && item->isSelected());
}

CardViewItem *CardView::selectedItem() const
{
  // find the first selected item
  QPtrListIterator<CardViewItem> iter(d->mItemList);
  for (iter.toFirst(); iter.current(); ++iter)
  {
    if ((*iter)->isSelected())
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

CardViewItem *CardView::findItem(const QString &text, const QString &label,
                                 Qt::StringComparisonMode compare)
{
  // IF the text is empty, we will return null, since empty text will
  // match anything!
  if (text.isEmpty())
    return 0;

  QPtrListIterator<CardViewItem> iter(d->mItemList);
  if (compare & Qt::BeginsWith)
  {
    QString value;
    for (iter.toFirst(); iter.current(); ++iter)
    {
      value = (*iter)->fieldValue(label).upper();
      if (value.startsWith(text.upper()))
        return *iter;
    }
  }
  else
  {
    kdDebug() << "CardView::findItem: search method not implemented" << endl;
  }

  return 0;
}

void CardView::viewportPaintEvent( QPaintEvent * )
{
  QPixmap pm( viewport()->width(), viewport()->height() );
  QPainter p;

  p.begin( &pm, viewport() );

  if (d->mLayoutDirty)
    calcLayout();

  QColorGroup cg = palette().active();
  pm.fill( cg.color( QColorGroup::Base ) );

  CardViewItem *item;
  CardViewSeparator *sep;

  // Now tell the cards to draw
  QPtrListIterator<CardViewItem> iter(d->mItemList);
  for (iter.toFirst(); iter.current(); ++iter)
  {
    item = *iter;
    QRect cardRect = item->d->mRect;

    // Tell the card to paint
    p.save();
    p.translate( cardRect.x() - contentsX(), cardRect.y() - contentsY() );
    item->paintCard( &p, cg );
    p.restore();
  }

  // Followed by the separators
  QPtrListIterator<CardViewSeparator> sepIter(d->mSeparatorList);
  for (sepIter.toFirst(); sepIter.current(); ++sepIter)
  {
    sep = *sepIter;
    QRect sepRect = sep->mRect;

    p.save();
    p.translate(sepRect.x() - contentsX(), sepRect.y() - contentsY() );
    sep->paintSeparator(&p, cg);
    p.restore();
  }

	p.end();
	bitBlt( viewport(), 0, 0, &pm );
}

void CardView::resizeEvent(QResizeEvent *e)
{
  QScrollView::resizeEvent(e);

  setLayoutDirty(true);
}

void CardView::calcLayout()
{
  //kdDebug() << "CardView::calcLayout:" << endl;

  // Start in the upper left corner and layout all the
  // cars using their height and width
  int maxWidth = 0;
  int maxHeight = 0;
  int xPos = 0;
  int yPos = 0;
  int cardSpacing = 10;

  // delete the old separators
  d->mSeparatorList.clear();

  QPtrListIterator<CardViewItem> iter(d->mItemList);
  CardViewItem *item = 0;
  CardViewSeparator *sep = 0;
  xPos += cardSpacing;

  for (iter.toFirst(); iter.current(); ++iter)
  {
    item = *iter;

    yPos += cardSpacing;

    if (yPos + item->d->mRect.height() + cardSpacing > height())
    {
      maxHeight = QMAX(maxHeight, yPos);

      // Drawing in this column would be greater than the height
      // of the scroll view, so move to next column
      yPos = cardSpacing;
      xPos += cardSpacing + maxWidth;
      if (d->mDrawSeparators)
      {
        // Create a separator since the user asked
        sep = new CardViewSeparator(this);
        sep->mRect.moveTopLeft(QPoint(xPos, 2*yPos));
        xPos += sep->mRect.width() + cardSpacing;
        d->mSeparatorList.append(sep);
      }

      maxWidth = 0;
    }

    item->d->mRect.moveTopLeft(QPoint(xPos, yPos));

    yPos += item->d->mRect.height();
    maxWidth = QMAX(maxWidth, item->d->mRect.width());
  }

  xPos += maxWidth;
  resizeContents(xPos + 10, viewport()->height());

  // Update the height of all the separators now that we know the
  // max height of a column
  QPtrListIterator<CardViewSeparator> sepIter(d->mSeparatorList);
  for (sepIter.toFirst(); sepIter.current(); ++sepIter)
  {
    (*sepIter)->mRect.setHeight(maxHeight - 4*cardSpacing);
  }

  d->mLayoutDirty = false;
}

CardViewItem *CardView::itemAfter(CardViewItem *item)
{
  int pos = d->mItemList.findRef(item);
  if ( pos == -1 )
      return 0L;

  return d->mItemList.at(pos+1);
}

void CardView::mousePressEvent(QMouseEvent *e)
{
  QScrollView::mousePressEvent(e);

  QPoint pos = e->pos();
  d->mLastClickPos = pos;

  CardViewItem *item = itemAt(viewportToContents(pos));

  if (item == 0)
  {
    d->mLastClickOnItem = false;
    selectAll(false);
    return;
  }

  d->mLastClickOnItem = true;

  // Always emit the selection
  emit clicked(item);

  // Check the selection type and update accordingly
  if (d->mSelectionMode == CardView::Single)
  {
    // make sure it isn't already selected
    if (item->isSelected())
      return;

    bool b = signalsBlocked();
    blockSignals(true);
    selectAll(false);
    blockSignals(b);

    item->setSelected(true);
    item->repaintCard();
    emit selectionChanged(item);
  }

  else if (d->mSelectionMode == CardView::Multi)
  {
    // toggle the selection
    item->setSelected(!item->isSelected());
    item->repaintCard();
    emit selectionChanged();
  }

  else if (d->mSelectionMode == CardView::Extended)
  {
    if ((e->button() & Qt::LeftButton) &&
        (e->state() & Qt::ControlButton))
    {
      item->setSelected(!item->isSelected());
      item->repaintCard();
      emit selectionChanged();
    }

    else if ((e->button() & Qt::LeftButton) &&
             (e->state() & Qt::ShiftButton))
    {
      kdDebug() << "CardView::mousePressEvent: shift-click not implemented"
                << endl;
    }

    else if (e->button() & Qt::LeftButton)
    {
      bool b = signalsBlocked();
      blockSignals(true);
      selectAll(false);
      blockSignals(b);

      item->setSelected(true);
      item->repaintCard();
      emit selectionChanged();
    }
  }

}

void CardView::mouseReleaseEvent(QMouseEvent *e)
{
  QScrollView::mouseReleaseEvent(e);

  // If there are accel keys, we will not emit signals
  if ((e->state() & Qt::ShiftButton) || (e->state() & Qt::ControlButton))
    return;

  // Get the item at this position
  CardViewItem *item = itemAt(viewportToContents(e->pos()));

  if (item && KGlobalSettings::singleClick())
  {
    emit executed(item);
  }
}

void CardView::mouseDoubleClickEvent(QMouseEvent *e)
{
  QScrollView::mouseDoubleClickEvent(e);

  CardViewItem *item = itemAt(viewportToContents(e->pos()));

  if (item && !KGlobalSettings::singleClick())
  {
    emit executed(item);
  }

  emit doubleClicked(item);
}

void CardView::setLayoutDirty(bool dirty)
{
  if (d->mLayoutDirty != dirty)
  {
    d->mLayoutDirty = dirty;
    repaint();
  }
}

void CardView::setDrawCardBorder(bool enabled)
{
  if (enabled != d->mDrawCardBorder)
  {
    d->mDrawCardBorder = enabled;
    repaint();
  }
}

bool CardView::drawCardBorder() const
{
  return d->mDrawCardBorder;
}

void CardView::setDrawColSeparators(bool enabled)
{
  if (enabled != d->mDrawSeparators)
  {
    d->mDrawSeparators = enabled;
    setLayoutDirty(true);
  }
}

bool CardView::drawColSeparators() const
{
  return d->mDrawSeparators;
}

void CardView::setDrawFieldLabels(bool enabled)
{
  if (enabled != d->mDrawFieldLabels)
  {
    d->mDrawFieldLabels = enabled;
    repaint();
  }
}

bool CardView::drawFieldLabels() const
{
  return d->mDrawFieldLabels;
}

void CardView::startDrag()
{
  // The default implementation is a no-op. It must be
  // reimplemented in a subclass to be useful
}

void CardView::mouseMoveEvent(QMouseEvent *e)
{
  if (d->mLastClickOnItem && (e->state() & Qt::LeftButton) &&
     ((e->pos() - d->mLastClickPos).manhattanLength() > 4))
     startDrag();
}

#include "cardview.moc"
