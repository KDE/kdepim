#ifndef __CERTBOX__H
#define __CERTBOX__H
#include <qlistview.h>

/**
   This is the listview that shows all the certificates.
*/
class CertBox :public QListView 
{
Q_OBJECT

public:
  CertBox( QWidget* parent, const char* name = 0 );

  void loadCerts();
protected slots:
  void handleDoubleClick( QListViewItem* item );
};

#endif // __CERTBOX__H
