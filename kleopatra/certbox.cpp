#include "certbox.h"
#include <klocale.h>
#include "certitem.h"

CertBox::CertBox( QWidget* parent, const char* name ) :QListView( parent, name )
{
  QFontMetrics fm = fontMetrics();
  addColumn( i18n("Subject"), fm.width( i18n("Subject") ) * 6  );
  addColumn( i18n("Issuer"), fm.width( i18n("Issuer") ) * 4 );
  addColumn( i18n("Serial")/*, fm.width( i18n("Serial") ) * 3*/ );
  setColumnWidthMode( 0, QListView::Manual );
  setColumnWidthMode( 1, QListView::Manual );
  setAllColumnsShowFocus( true );

  connect( this, SIGNAL( doubleClicked (QListViewItem*) ), this, SLOT( handleDoubleClick( QListViewItem*) ) );
  connect( this, SIGNAL( returnPressed (QListViewItem*) ), this, SLOT( handleDoubleClick( QListViewItem*) ) );
}

void CertBox::handleDoubleClick( QListViewItem* item ) 
{
  static_cast<CertItem*>(item)->display();
}

void CertBox::loadCerts()
{

}

#include "certbox.moc"
