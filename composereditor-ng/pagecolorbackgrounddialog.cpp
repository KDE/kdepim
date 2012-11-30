#include "pagecolorbackgrounddialog.h"
#include "ui_pagecolorbackgrounddialog.h"

PageColorBackgroundDialog::PageColorBackgroundDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageColorBackgroundDialog)
{
    ui->setupUi(this);
}

PageColorBackgroundDialog::~PageColorBackgroundDialog()
{
    delete ui;
}
