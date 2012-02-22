#ifndef SELECTCOMPONENTPAGE_H
#define SELECTCOMPONENTPAGE_H

#include <QWidget>

namespace Ui {
  class SelectComponentPage;
}

class SelectComponentPage : public QWidget
{
  Q_OBJECT
  
public:
  explicit SelectComponentPage(QWidget *parent = 0);
  ~SelectComponentPage();
  
private:
  Ui::SelectComponentPage *ui;
};

#endif // SELECTCOMPONENTPAGE_H
