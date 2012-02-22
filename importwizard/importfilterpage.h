#ifndef IMPORTFILTERPAGE_H
#define IMPORTFILTERPAGE_H

#include <QWidget>

namespace Ui {
  class ImportFilterPage;
}

class ImportFilterPage : public QWidget
{
  Q_OBJECT
  
public:
  explicit ImportFilterPage(QWidget *parent = 0);
  ~ImportFilterPage();
  
private:
  Ui::ImportFilterPage *ui;
};

#endif // IMPORTFILTERPAGE_H
