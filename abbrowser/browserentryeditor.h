#ifndef PABCONTACT_H 
#define PABCONTACT_H 

#include "entryeditorwidget.h"

class KAB::Entity;
class QWidget;

class PabContactDialog : public ContactDialog
{
  Q_OBJECT

public:
  PabContactDialog::PabContactDialog( QWidget *parent, 
				      const char *name,
				      QString entryKey,
              KAB::Entity* entry,
				      bool modal = FALSE );
  virtual ~PabContactDialog();
 
signals:
  virtual void change( QString entryKey , KAB::Entity *ce );

protected slots:
  virtual void accept();

protected:
  QString entryKey_; 
};

class PabNewContactDialog : public ContactDialog
{
  Q_OBJECT

public:
  PabNewContactDialog::PabNewContactDialog( QWidget *parent, 
					    const char *name, 
					    bool modal = FALSE );
  virtual ~PabNewContactDialog();

signals:
  virtual void add( KAB::Entity* ce );

protected slots:
  virtual void accept();
};

#endif // PABWIDGET_H 
