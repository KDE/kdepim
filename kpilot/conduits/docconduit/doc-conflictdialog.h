/****************************************************************************
** Form interface generated from reading ui file './doc-conflictdialog.ui'
**
** Created: Son Dez 29 17:51:44 2002
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;

class ResolutionDialog : public KDialogBase
{
    Q_OBJECT

public:
    ResolutionDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ResolutionDialog();

    QGroupBox* groupBox1;
    QLabel* textLabel2;
    QComboBox* fDBResolution_1;
    QComboBox* fDBResolution_2;
    QLabel* textLabel2_2;
    QPushButton* pushButton1_2;
    QPushButton* pushButton1;
    QLabel* textLabel1;
    QLabel* textLabel1_2;

protected:
    QGridLayout* Form1Layout;
    QGridLayout* groupBox1Layout;

protected slots:
    virtual void languageChange();
private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;

};

#endif // FORM1_H
