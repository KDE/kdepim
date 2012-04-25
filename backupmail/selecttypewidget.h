#ifndef SELECTTYPEWIDGET_H
#define SELECTTYPEWIDGET_H

#include <QWidget>

namespace Ui {
  class SelectTypeWidget;
}

class SelectTypeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit SelectTypeWidget(QWidget *parent = 0);
  ~SelectTypeWidget();
  
private:
  Ui::SelectTypeWidget *ui;
};

#endif // SELECTTYPEWIDGET_H
