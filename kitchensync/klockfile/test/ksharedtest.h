/****************************************************************************
** Form interface generated from reading ui file './ksharedtest.ui'
**
** Created: Wed Feb 6 23:10:15 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <ksharedfile.h>
#include <qmainwindow.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QLineEdit;
class QPushButton;
class QToolButton;

class Form1 : public QMainWindow
{ 
    Q_OBJECT

public:
    Form1( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~Form1();

    QToolButton* ToolButton1;
    QLineEdit* LineEdit1;
    QPushButton* PushButton3;
    QPushButton* PushButton2;
    QPushButton* PushButton4;
    QPushButton* PushButton1;
    QAction* Action;


public slots:
    virtual void slotWriteUnlock();
    virtual void slotReadLock();
    virtual void slotReadUnlock();
    virtual void slotWriteLock();
    virtual void init();

protected:

    KSharedFile::Ticket *ticket;
    KSharedFile *file;
};

#endif // FORM1_H
