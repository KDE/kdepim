#include "importsettingpage.h"
#include "ui_importsettingpage.h"

ImportSettingPage::ImportSettingPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ImportSettingPage)
{
  ui->setupUi(this);
}

ImportSettingPage::~ImportSettingPage()
{
  delete ui;
}

#include "importsettingpage.moc"
