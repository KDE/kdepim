#include "certitem.h"
#include <kdialogbase.h>
#include <qvbox.h>
#include <qlabel.h>
#include <klocale.h>
#include "agent.h"
#include <qframe.h>

#include "certificateinfowidgetimpl.h"

CertItem::CertItem( const QString& DN, 
		    const QString& serial,
		    const QString& issuer, 
		    const QString& CN, 
		    const QString& L,
		    const QString& O,
		    const QString& OU,
		    const QString& C, 
		    const QString& email, 
		    const QDateTime& created,
		    const QDateTime& expire,
		    bool sign,
		    bool encrypt,
		    bool certify,
		    const CryptPlugWrapper::CertificateInfo& info,
		    Agent* agent,CertManager* manager, CertBox* parent )
  : QListViewItem( parent, DN, issuer, serial /*agent?agent->shortName():""*/) , 
   _DN(DN), _serial(serial),_issuer(issuer), _CN(CN),_L(L), _O(O), _OU(OU), _C(C), _email(email),
    _created(created),_expire(expire),
    _sign(sign),_encrypt(encrypt),_certify(certify),
    _info(info),
    _agent(agent),
    _manager(manager)
{
  init();
}

CertItem::CertItem( const QString& DN, 
		    const QString& serial,
		    const QString& issuer, 
		    const QString& CN, 
		    const QString& L,
		    const QString& O,
		    const QString& OU,
		    const QString& C, 
		    const QString& email, 
		    const QDateTime& created,
		    const QDateTime& expire,
		    bool sign,
		    bool encrypt,
		    bool certify,
		    const CryptPlugWrapper::CertificateInfo& info,
		    Agent* agent,CertManager* manager, CertItem* parent )
  : QListViewItem( parent, DN, issuer, serial /*agent?agent->shortName():"" */), 
   _DN(DN), _serial(serial), _issuer(issuer), _CN(CN),_L(L), _O(O), _OU(OU), _C(C), _email(email), 
    _created(created),_expire(expire),
    _sign(sign),_encrypt(encrypt),_certify(certify),
    _info(info),
    _agent(agent),
    _manager(manager)
{
  init();
}

void CertItem::init()
{
  setOpen( true );
  setText( 3, _created.toString() );
  setText( 4, _expire.toString() );
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

  CertificateInfoWidgetImpl* top = new CertificateInfoWidgetImpl( _manager, dialog );
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
