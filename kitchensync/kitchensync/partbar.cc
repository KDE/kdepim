
#include <kiconloader.h>

#include "partbar.h"

using namespace KitchenSync;


PartBarItem::PartBarItem( PartBar *parent, ManipulatorPart *part ) 
  : QListBoxPixmap(KIconLoader::unknown() )
{
  m_Parent = parent;
  m_Part = part;
  setCustomHighlighting( true );
  setText( part->description() ); 
}
PartBarItem::~PartBarItem()
{

}
ManipulatorPart* PartBarItem::part()
{
  return m_Part;
}
QString PartBarItem::toolTip() const
{

}
int PartBarItem::iconSize()const
{

}
int PartBarItem::width( const QListBox * ) const
{

}
int PartBarItem::height( const QListBox *) const
{

}
int PartBarItem::iconSize() const
{
  return m_parent->iconSize();
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
  if ( isCurrent() || isSelected() ) {
    qDrawShadePanel( p, 1, 0, w -2, height(box),
		     box->colorGroup(), true, 1, 0L );
  }
}


PartBar::PartBar(QWidget *parent)
