#ifndef __CERTITEM_H
#define __CERTITEM_H
#include <qlistview.h>
#include "certbox.h"
#include <qvaluelist.h>
#include <qpair.h>
#include <qdatetime.h>
#include <cryptplugwrapper.h>
class Agent;
class CertManager;

/**
   One item in the certificate list view.
*/
class CertItem :public QListViewItem 
{
public:
  CertItem( const QString& DN, 
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
	    Agent* agent, CertManager* manager, CertBox* parent );
  CertItem( const QString& DN, 
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
	    Agent* agent, CertManager* manager, CertItem* parent );
  void addKey( const QString& key, const QString& value );
  void display();

  QString dn() const { return _DN; }

private:
  void init();

  QString _DN;
  QString _serial;

  QString _issuer;

  QString _CN;
  QString _L;
  QString _O;
  QString _OU;
  //QString _ST;
  QString _C;
  QString _email;

  QDateTime _created;
  QDateTime _expire;

  bool _sign;
  bool _encrypt;
  bool _certify;

  CryptPlugWrapper::CertificateInfo _info;
  
  Agent* _agent;
  CertManager* _manager;
  QValueList< QPair<QString,QString> > _extras;
};

#endif // __CERTITEM_H
