#include <ktmainwindow.h>
#include <kbuttonbox.h>
#include <qpushbutton.h>
#include "contact.h"
#include <qlayout.h>

class AddressBookDialog : public KTMainWindow
{
    Q_OBJECT

    public:
    
        AddressBookDialog(ContactEntry * e, const char * name = 0);

        virtual ~AddressBookDialog();
  
    private:
        
        QWidget             * mainWidget_;
        QGridLayout         * layout_;
        ContactDialog       * abWidget_;
        KButtonBox          * buttonBox_;
        QPushButton         * pb_OK_;
        QPushButton         * pb_apply_;
        QPushButton         * pb_cancel_;
        QPushButton         * pb_defaults_;
        QPushButton         * pb_help_;
};


