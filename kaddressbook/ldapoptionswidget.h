#ifndef LDAPOPTIONSWIDGET_H
#define LDAPOPTIONSWIDGET_H

#include <qwidget.h>

class KListView;
class QPushButton;

class LDAPOptionsWidget : public QWidget
{ 
  Q_OBJECT

  public:
    LDAPOptionsWidget( QWidget* parent = 0, const char* name = 0 );
    ~LDAPOptionsWidget();

    void restoreSettings();
    void saveSettings();

  public slots:
    void slotAddHost();
    void slotEditHost();
    void slotRemoveHost();
    void slotSelectionChanged( QListViewItem* );

  private:
    void initGUI();

    KListView* mHostListView;

    QPushButton* mAddButton;
    QPushButton* mEditButton;
    QPushButton* mRemoveButton;
};

#endif // LDAPOPTIONSWIDGET_H
