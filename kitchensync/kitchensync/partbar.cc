
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
QString PartBarItem::toolTip()
{

}
int PartBarItem::width( const QListBox * ) const
{

}
int PartBarItem::height( const QListBox *) const
{

}

PartBar::PartBar(QWidget *parent)
