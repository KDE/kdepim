#include "importmailpage.h"
#include "ui_importmailpage.h"

ImportMailPage::ImportMailPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ImportMailPage)
{
  ui->setupUi(this);
}

ImportMailPage::~ImportMailPage()
{
  delete ui;
}

#include "importmailpage.moc"
