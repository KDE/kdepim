#ifndef SELECTFIELDSWIDGET_H 
#define SELECTFIELDSWIDGET_H 

#include <qdialog.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kabc/field.h>
#include <kabc/addressbook.h>

class QListBox;
class QListBoxItem;
class QLineEdit;
class QComboBox;
class QPushButton;
class QToolButton;

class SelectFieldsWidget : public QWidget
{
  Q_OBJECT

  public:
    SelectFieldsWidget( KABC::AddressBook *doc,
                        const KABC::Field::List &oldFields,
                        QWidget *parent = 0, const char *name = 0 );
                       
    SelectFieldsWidget( KABC::AddressBook *doc, QWidget *parent = 0,
                        const char *name = 0);
    
    virtual void setOldFields( const KABC::Field::List & );
    virtual KABC::Field::List chosenFields();

  public slots:
    virtual void select();
    virtual void unselect();
    virtual void addCustom();
    virtual void showFields( int );
    virtual void textChanged(const QString &);
  
    /* sets move buttons enabled according to selection */
    void setButtonsEnabled();
    void setButtonsEnabled( QListBoxItem * ) { setButtonsEnabled(); };
  
    /* move current item in selected list one place up */
    void moveUp();
  
    /* move current item in selected list one place down */
    void moveDown();
  
  private:
    void initGUI( KABC::AddressBook * );
    
    QListBox *lbUnSelected;
    QListBox *lbSelected;
    QLineEdit *leCustomField;
    QComboBox *cbUnselected;
    QPushButton *pbAddCustom;
    /*QPushButton *pbAdd;
    QPushButton *pbRemove;*/
    QToolButton *pbAdd, *pbRemove, *pbUp, *pbDown;
    
    KABC::AddressBook *mDoc;
};

#endif // SELECTFIELDSWIDGET_H 
