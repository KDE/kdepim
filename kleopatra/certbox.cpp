#include "certbox.h"
#include <klocale.h>
#include "certitem.h"

CertBox::CertBox( QWidget* parent, const char* name ) :QListView( parent, name )
{
  addColumn( i18n("Subject") );
  addColumn( i18n("Issuer") );

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
