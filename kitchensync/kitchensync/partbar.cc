#include <qapplication.h>
#include <qfontmetrics.h>
#include <qpainter.h>

#include <kglobal.h>
#include <kiconloader.h>

#include "partbar.h"

using namespace KitchenSync;

PartBarItem::PartBarItem( PartBar *parent, ManipulatorPart *part ) 
  : QListBoxPixmap(KIconLoader::unknown() ) {
  m_Parents = parent;
  m_Part = part;
  m_Pixmap = m_Part->pixmap();
  setCustomHighlighting( true );
  setText( part->description() ); 
  setTooltip(part->description() );
}

PartBarItem::~PartBarItem() {
}

ManipulatorPart* PartBarItem::part() {
  return m_Part;
}

QString PartBarItem::toolTip() const {
  return ( m_Part->description() );
}

int PartBarItem::width( const QListBox *listbox) const {
  return listbox->viewport()->width() :
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


PartBar::PartBar(QWidget *parent) 
  : QFrame ( parent ),
    m_activeItem ( OL ),
    m_listBox( OL ) {
  
  setListBox( OL );
  setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred ) );
}

PartBarItem * PartBar::inserItem( ManipulatorPart *part) {
  PartBarItem *item = new PartBarItem( this, part );
  item->SetApplicationLocal ( applicationLocal );
  return item;
}

void PartBar::setListBox(KListBox *view) {
  delete m_listBox;
  
  if ( !view ) {
    m_listBox = new KListBox(this);
  } else {
    m_listBox = view;
    if ( m_listBox->parentWidget() != this ) {
      m_listBox->reparen( this, QPoint(0,0) );
    }
    m_listBox->resize( width(), height() );
  }
  
  m_listBox->setSelectionMode( KListBox::Single );
  QPalette pal = palette();
  QColor gray = pal.color(QPalette::Normal, QColorGroup::Mid );
  pal.setColor( QPalette::Normal, QColorGroup::Base, gray );
  pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );
  
  setPalette( pal );
  m_listBox->viewport()->setBackgroudMode( PaletteMid);

  connect( m_listBox, SIGNAL( clicked (QListBoxItem *)),
	   SLOT( slotSelected( QListBoxItem * )));
}

void PartBar::clear() {
  m_listBox->clear();
}

void PartBar::resizeEvent() {
  QFrame::resizeEvent( e );
  m_listBox->resize( width(), height() );
}

QSize PartBar::sizeHint() {
  int w = 0;
  int h = 0;
  
  QListBoxItem *item;
  
  for ( item = m_listBox->firstItem(); item; item = item->next() ) {
    w = QMAX(w , item->width( m_listBox ));
    h += item->height( m_listBox );
  }

  if (m_listBox->verticalScroolBar->isVisible() ) {
    w += m_listBox->verticalScrollBar()->width();
  }

  if ( W == 0 && h == 0) {
    return QSize(100, 200);
  } else {
    return QSize( 6 + w , h );
  }
}

QSize PartBar::minimumSizeHint() {
  QSize s = sizeHint();
  int h = s.height() + m_listBox->horizontalScrollBar()->height();
  int w = s.width() + m_listBox->verticalScroolBar()->width();
  return QSize( w, h );
}

void PartBar::slotSelected(QListBoxItem *item) {
  if ( item && item != m_activeItem ) {
    PartBarItem* it = static_cast<PartBarItem*>( item );
    m_activeItem = it;
    emit activated( it->part() );
  }
}

ParBarItem * PartBar::currentItem() const {
  QListBoxItem *item = m_listBox->item(m_listBox->currentItem() );
  if ( item ) {
    return static_cast<PartBarItem *>( item );
  } else {
    return OL;
  }
}



