#ifndef KABC_ADDRESSEEDIALOG_H
#define KABC_ADDRESSEEDIALOG_H

#include <qdict.h>

#include <kdialogbase.h>
#include <klistview.h>
#include <klineedit.h>

#include "addressbook.h"

namespace KABC {

class AddresseeDialog : public KDialogBase {
    Q_OBJECT
  public:
    AddresseeDialog( QWidget *parent );
    virtual ~AddresseeDialog();

    static Addressee getAddressee( QWidget *parent );

  private slots:
    void selectItem( const QString & );
    void updateEdit( QListViewItem *item );

  private:
    void loadAddressBook();
    void addCompletionItem( const QString &str, QListViewItem *item );
  
    KListView *mAddresseeList;
    KLineEdit *mAddresseeEdit;
    
    AddressBook *mAddressBook;
    
    QDict<QListViewItem> mItemDict;
};

}

#endif
