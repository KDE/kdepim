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
  CertItem( const CryptPlugWrapper::CertificateInfo& info,
	    Agent* agent, CertManager* manager, CertBox* parent );
  CertItem( const CryptPlugWrapper::CertificateInfo& info,
	    Agent* agent, CertManager* manager, CertItem* parent );
  void addKey( const QString& key, const QString& value );
  void display();

  QString dn() const { return _info.userid[0].stripWhiteSpace(); }
  QString issuer() const {  return _info.issuer.stripWhiteSpace(); }
  QString serial() const {  return _info.serial; }
  QDateTime created() const { return _info.created; }
  QDateTime expire() const { return _info.expire; }

  virtual QString text(int) const;

private:
  void init();

  CryptPlugWrapper::CertificateInfo _info;
  
  Agent* _agent;
  CertManager* _manager;
  QValueList< QPair<QString,QString> > _extras;
};

#endif // __CERTITEM_H
