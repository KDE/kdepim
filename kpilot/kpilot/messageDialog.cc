#include <qdialog.h>
#include <kapp.h>
#include "messageDialog.moc"

MessageDialog::MessageDialog( const char *title, QWidget* parent, const char* name, bool modal)
  : QDialog(parent, name, modal, 0)
    {
    setGeometry(x(), y(), 250, 40);
    setCaption(title);
    fMessage = new QLabel(title, this);
    fMessage->setFixedWidth(220);
    fMessage->move(10, 10);
    fMessage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    fMessage->setAlignment(AlignBottom | AlignCenter);
    kapp->processEvents();
    }
  
void 
MessageDialog::setMessage(const char* message)
    {
    fMessage->setText(message);
    fMessage->adjustSize();
    kapp->processEvents();
    }
