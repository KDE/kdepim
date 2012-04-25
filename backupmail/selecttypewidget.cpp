#include "selecttypewidget.h"
#include "ui_selecttypewidget.h"

SelectTypeWidget::SelectTypeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SelectTypeWidget)
{
  ui->setupUi(this);
}

SelectTypeWidget::~SelectTypeWidget()
{
  delete ui;
}
