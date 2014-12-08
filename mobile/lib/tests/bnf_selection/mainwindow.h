
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
class QStandardItemModel;
class KBreadcrumbNavigationFactory;

class MainWindow : public QWidget
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = Q_NULLPTR);

private Q_SLOTS:
  void doSelects();

private:
  KBreadcrumbNavigationFactory *m_bnf;
  QStandardItemModel *m_model;

};

#endif

