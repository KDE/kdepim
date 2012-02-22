#include "importfilterpage.h"
#include "ui_importfilterpage.h"

ImportFilterPage::ImportFilterPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ImportFilterPage)
{
  ui->setupUi(this);
}

ImportFilterPage::~ImportFilterPage()
{
  delete ui;
}

#include "importfilterpage.moc"
