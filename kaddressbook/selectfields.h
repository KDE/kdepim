#ifndef SELECTFIELDS_H 
#define SELECTFIELDS_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qstringlist.h>
#include <qwidget.h>
#include <kdialogbase.h>

class SelectFieldsWidget;

class SelectFields : public KDialogBase
{
  Q_OBJECT

  public:
    SelectFields(QStringList oldFields,
	               QWidget *parent = 0, 
		             const char *name = 0, 
		             bool modal = false );
    SelectFields(QWidget *parent = 0, 
		            const char *name = 0, 
		            bool modal = false );
    virtual ~SelectFields() {}
               
    virtual void setOldFields(const QStringList &fields);
    virtual QStringList chosenFields();

  private:
    SelectFieldsWidget *mWidget;
};

#endif // PABWIDGET_H 
