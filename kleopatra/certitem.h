#ifndef __CERTITEM_H
#define __CERTITEM_H
#include <qlistview.h>
#include "certbox.h"
#include <qvaluelist.h>
#include <qpair.h>
class Agent;

/**
   One item in the certificate list view.
*/
class CertItem :public QListViewItem 
{
public:
  CertItem( const QString& DN, 
	    const QString& issuer,
	    const QString& CN, 
	    const QString& L,
	    const QString& O,
	    const QString& OU,
	    const QString& C, 
	    const QString& email,
	    Agent* agent, CertBox* parent );
  void addKey( const QString& key, const QString& value );
  void display();

private:
  QString _DN;
  QString _issuer;

  QString _CN;
  QString _L;
  QString _O;
  QString _OU;
  //QString _ST;
  QString _C;
  QString _email;
  
  Agent* _agent;
  QValueList< QPair<QString,QString> > _extras;
};

#endif // __CERTITEM_H
