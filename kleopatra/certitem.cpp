#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>

#include <klocale.h>
#include <kdialogbase.h>

#include "agent.h"
#include "certitem.h"
#include "certmanager.h"

#include "certificateinfowidgetimpl.h"

CertItem::CertItem( const CryptPlugWrapper::CertificateInfo& info,
		    Agent* agent,CertManager* manager, CertBox* parent )
  : QListViewItem( parent ) , 
    _info(info),
    _agent(agent),
    _manager(manager)
{
  init();
}

CertItem::CertItem( const CryptPlugWrapper::CertificateInfo& info,
		    Agent* agent,CertManager* manager, CertItem* parent )
  : QListViewItem( parent ), 
    _info(info),
    _agent(agent),
    _manager(manager)
{
  init();
}

void CertItem::init()
{
  setOpen( true );
}

QString CertItem::text( int col ) const
{
  switch( col ) {
  case 0:
    return dn();
  case 1:
    return issuer();
  case 2:
    return serial();
  case 3:
    return created().toString();
  case 4:
    return expire().toString();
  default:
    return QListViewItem::text(col);
  }
}

/**
   Certificates may have a variable number of keys, some of these may be dublicates,
   an example of this is that a certificate may have several email addresses.
   This method allows for adding an arbitrary key.
*/
void CertItem::addKey( const QString& key, const QString& value )
{
  _extras.append( QPair<QString,QString>(key, value) );
}

/**
   This method is invoked when the user double clicks on a certificate, and thus wants to
   see its details.
*/
void CertItem::display() 
{
  KDialogBase* dialog = new KDialogBase( listView(), "dialog", true, i18n("Additional Information for Key"), KDialogBase::Close, KDialogBase::Close );

  CertificateInfoWidgetImpl* top = new CertificateInfoWidgetImpl( _manager, _manager->isRemote(), dialog );
  dialog->setMainWidget( top );
  top->setCert( _info ); 
#if 0
  // Extra Keys
  for ( QValueList< QPair<QString,QString> >::iterator it = _extras.begin(); it != _extras.end(); ++it ) {
    new QLabel( QString("%1: %2").arg( (*it).first ).arg( (*it).second ), top );
  }

  // Agent Information
  QFrame* frame = new QFrame(top);
  frame->setFrameStyle( QFrame::HLine | QFrame:: Plain );
  
  new QLabel(i18n("<b>CA information</b>"), top);
  //new QLabel( _agent->tree(), top );
  //if( parent() ) new QLabel( static_cast<CertItem*>(parent())->dn(), top );
  new QLabel(i18n("Issued by: %1").arg( _issuer ), top );
#endif
  
  dialog->exec();
  delete dialog;
}
