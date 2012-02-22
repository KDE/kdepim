#ifndef IMPORTMAILPAGE_H
#define IMPORTMAILPAGE_H

#include <QWidget>

namespace Ui {
  class ImportMailPage;
}

class ImportMailPage : public QWidget
{
  Q_OBJECT
  
public:
  explicit ImportMailPage(QWidget *parent = 0);
  ~ImportMailPage();
  
private:
  Ui::ImportMailPage *ui;
};

#endif // IMPORTMAILPAGE_H
