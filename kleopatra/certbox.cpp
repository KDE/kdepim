#include "certbox.h"
#include <klocale.h>
#include "certitem.h"

CertBox::CertBox( QWidget* parent, const char* name ) :QListView( parent, name )
{
  setRootIsDecorated( true );
  QFontMetrics fm = fontMetrics();
  addColumn( i18n("Subject"), fm.width( i18n("Subject") ) * 5  );
  addColumn( i18n("Issuer"), fm.width( i18n("Issuer") ) * 3 );
  addColumn( i18n("Serial")/*, fm.width( i18n("Serial") ) * 3*/ );
  setColumnWidthMode( 0, QListView::Manual );
  setColumnWidthMode( 1, QListView::Manual );
  //setColumnWidthMode( 3, QListView::Manual );

  connect( this, SIGNAL( doubleClicked (QListViewItem*) ), this, SLOT( handleDoubleClick( QListViewItem*) ) );
}

void CertBox::handleDoubleClick( QListViewItem* item ) 
{
  static_cast<CertItem*>(item)->display();
}

void CertBox::loadCerts()
{

}

#include "certbox.moc"
