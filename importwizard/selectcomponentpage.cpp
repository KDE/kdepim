#include "selectcomponentpage.h"
#include "ui_selectcomponentpage.h"

SelectComponentPage::SelectComponentPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SelectComponentPage)
{
  ui->setupUi(this);
}

SelectComponentPage::~SelectComponentPage()
{
  delete ui;
}

#include "selectcomponentpage.moc"
