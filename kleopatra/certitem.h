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
  CertItem( const QString& DN, const QString& O, const QString& C, Agent* agent, CertBox* parent );
  void addKey( const QString& key, const QString& value );
  void display();

private:
  QString _DN;
  QString _O;
  QString _C;
  Agent* _agent;
  QValueList< QPair<QString,QString> > _extras;
};

#endif // __CERTITEM_H
