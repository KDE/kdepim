#include "certitem.h"
#include <kdialogbase.h>
#include <qvbox.h>
#include <qlabel.h>
#include <klocale.h>
#include "agent.h"
#include <qframe.h>

CertItem::CertItem( const QString& DN, 
		    const QString& issuer, 
		    const QString& CN, 
		    const QString& L,
		    const QString& O,
		    const QString& OU,
		    const QString& C, 
		    const QString& email, 
		    Agent* agent, CertBox* parent )
  : QListViewItem( parent, DN, agent->shortName() ), 
   _DN(DN), _issuer(issuer), _CN(CN),_L(L), _O(O), _OU(OU), _C(C), _email(email), _agent(agent)
{
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
  QVBox* top = new QVBox( dialog );
  top->setSpacing(6);
  
  dialog->setMainWidget( top );
  
  // Fixed Keys
  new QLabel(i18n("<b>Certificate Information</b>"), top );
  new QLabel(i18n("DN: %1").arg( _DN ), top );
  new QLabel(i18n("CN: %1").arg( _CN ), top );
  new QLabel(i18n("L: %1").arg( _L ), top );
  new QLabel(i18n("O: %1").arg( _O ), top );
  new QLabel(i18n("OU: %1").arg( _OU ), top );
  new QLabel(i18n("C: %1").arg( _C ), top );
  new QLabel(i18n("Email: %1").arg( _email ), top );

  new QLabel(i18n("Issued by: %1").arg( _issuer ), top );
 
  // Extra Keys
  for ( QValueList< QPair<QString,QString> >::iterator it = _extras.begin(); it != _extras.end(); ++it ) {
    new QLabel( QString("%1: %2").arg( (*it).first ).arg( (*it).second ), top );
  }

  // Agent Information
  QFrame* frame = new QFrame(top);
  frame->setFrameStyle( QFrame::HLine | QFrame:: Plain );
  
  new QLabel(i18n("<b>CA information</b>"), top);
  new QLabel( _agent->tree(), top );
  
  dialog->exec();
  delete dialog;
}
