#include "importaddressbookpage.h"
#include "ui_importaddressbookpage.h"

ImportAddressbookPage::ImportAddressbookPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ImportAddressbookPage)
{
  ui->setupUi(this);
}

ImportAddressbookPage::~ImportAddressbookPage()
{
  delete ui;
}

#include "importaddressbookpage.moc"
