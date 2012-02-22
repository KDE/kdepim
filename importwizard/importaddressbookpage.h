#ifndef IMPORTADDRESSBOOKPAGE_H
#define IMPORTADDRESSBOOKPAGE_H

#include <QWidget>

namespace Ui {
  class ImportAddressbookPage;
}

class ImportAddressbookPage : public QWidget
{
  Q_OBJECT
  
public:
  explicit ImportAddressbookPage(QWidget *parent = 0);
  ~ImportAddressbookPage();
  
private:
  Ui::ImportAddressbookPage *ui;
};

#endif // IMPORTADDRESSBOOKPAGE_H
