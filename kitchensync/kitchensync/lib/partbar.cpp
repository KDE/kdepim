/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <qpainter.h>
#include <qdrawutil.h>

#include <kiconloader.h>
#include <kdebug.h>

#include "manipulatorpart.h"
#include "partbar.h"

using namespace KSync;

PartBarItem::PartBarItem( PartBar *parent, ManipulatorPart *part )
  : QListBoxPixmap(KIconLoader::unknown() ) {
  m_Parents = parent;
  m_Part = part;
  m_Pixmap = m_Part->pixmap();
  setCustomHighlighting( true );
  setText( part->name() );
  //tooltip(part->description() );
}

PartBarItem::~PartBarItem() {
}

ManipulatorPart* PartBarItem::part() {
  return m_Part;
}

//QString PartBarItem::toolTip() const {
//  return ( m_Part->description() );
//}

int PartBarItem::width( const QListBox *listbox) const {
  return listbox->viewport()->width();
}

int PartBarItem::height( const QListBox *listbox) const {
  int min = 0;
  min = listbox->fontMetrics().lineSpacing() + pixmap()->height() + 6;
  return min;
}

void PartBarItem::paint(QPainter *p)
{
  QListBox *box = listBox();
  int w = width( box );
  static const int margin = 3;
  int y = margin;
  const QPixmap *pm = pixmap();

  if ( !pm->isNull() ) {
    int x = (w - pm->width()) / 2;
    x = QMAX( x, margin );
    p->drawPixmap( x, y, *pm );
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = p->fontMetrics();
    y += pm->height() + fm.height() - fm.descent();
    int x = (w - fm.width( text() )) / 2;
    x = QMAX( x, margin );
    p->drawText( x, y, text() );
  }
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    qDrawShadePanel( p, 1, 0, w -2, height(box),
		     box->colorGroup(), true, 1, 0L );
  }
}


PartBar::PartBar(QWidget *parent, const char *name, WFlags f)
  : QFrame ( parent, name, f ),
    m_listBox( 0L ),
    m_activeItem ( 0L ) {

  setListBox( 0L );
  setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
}

PartBarItem * PartBar::insertItem( ManipulatorPart *part, int pos ) {
//  kdDebug(5210) << part->name() << "\n" << part->description() << "\n";
  PartBarItem *item = new PartBarItem( this , part );
  m_listBox->insertItem( item, pos );
  return item;
}

void PartBar::setListBox(KListBox *view) {
  delete m_listBox;

  if ( !view ) {
    m_listBox = new KListBox(this);
  } else {
    m_listBox = view;
    if ( m_listBox->parentWidget() != this ) {
      m_listBox->reparent( this, QPoint(0,0) );
    }
    m_listBox->resize( width(), height() );
  }

  m_listBox->setSelectionMode( KListBox::Single );
  QPalette pal = palette();
  QColor gray = pal.color(QPalette::Normal, QColorGroup::Mid );
  pal.setColor( QPalette::Normal, QColorGroup::Base, gray );
  pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );

  setPalette( pal );
  m_listBox->viewport()->setBackgroundMode( PaletteMid);

  connect( m_listBox, SIGNAL( clicked (QListBoxItem *)),
	   SLOT( slotSelected( QListBoxItem * )));
}

void PartBar::clear() {
  m_listBox->clear();
}

void PartBar::resizeEvent( QResizeEvent *e ) {
  QFrame::resizeEvent( e );
  m_listBox->resize( width(), height() );
}

QSize PartBar::sizeHint() const {

  int w = 0;
  int h = 0;

  QListBoxItem *item;

  for ( item = m_listBox->firstItem(); item; item = item->next() ) {
    w = QMAX(w , item->width( m_listBox ));
    h += item->height( m_listBox );
  }

  if (m_listBox->verticalScrollBar()->isVisible() ) {
    w += m_listBox->verticalScrollBar()->width();
  }

  if ( w == 0 && h == 0) {
    return QSize(100, 200);
  } else {
    return QSize( 6 + w , h );
  }
}

QSize PartBar::minimumSizeHint() const {
  QSize s = sizeHint();
  int h = s.height() + m_listBox->horizontalScrollBar()->height();
  int w = s.width() + m_listBox->verticalScrollBar()->width();
  return QSize( w, h );
}

void PartBar::slotSelected(QListBoxItem *item) {
  if ( item && item != m_activeItem ) {
    PartBarItem* it = static_cast<PartBarItem*>( item );
    m_activeItem = it;
    emit activated( it->part() );
  }
}

PartBarItem * PartBar::currentItem() const {
  QListBoxItem *item = m_listBox->item(m_listBox->currentItem() );
  if ( item ) {
    return static_cast<PartBarItem *>( item );
  } else {
    return 0L;
  }
}

#include "partbar.moc"
