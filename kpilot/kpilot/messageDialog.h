#ifndef __MESSAGE_DIALOG
#define __MESSAGE_DIALOG

#include <qdialog.h>
#include <qlabel.h>

class MessageDialog : public QDialog
    {
    Q_OBJECT

    public:
    MessageDialog( QString title, QWidget* parent=0, const char* name=0, bool modal=false);
    void setMessage(QString  message);
    
    protected:
    QLabel* fMessage;
    };
#endif
