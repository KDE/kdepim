#ifndef IMPORTSETTINGPAGE_H
#define IMPORTSETTINGPAGE_H

#include <QWidget>

namespace Ui {
  class ImportSettingPage;
}

class ImportSettingPage : public QWidget
{
  Q_OBJECT
  
public:
  explicit ImportSettingPage(QWidget *parent = 0);
  ~ImportSettingPage();
  
private:
  Ui::ImportSettingPage *ui;
};

#endif // IMPORTSETTINGPAGE_H
