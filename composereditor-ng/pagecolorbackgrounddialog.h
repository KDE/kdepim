#ifndef PAGECOLORBACKGROUNDDIALOG_H
#define PAGECOLORBACKGROUNDDIALOG_H

#include <QWidget>

namespace Ui {
class PageColorBackgroundDialog;
}

class PageColorBackgroundDialog : public QWidget
{
    Q_OBJECT
    
public:
    explicit PageColorBackgroundDialog(QWidget *parent = 0);
    ~PageColorBackgroundDialog();
    
private:
    Ui::PageColorBackgroundDialog *ui;
};

#endif // PAGECOLORBACKGROUNDDIALOG_H
