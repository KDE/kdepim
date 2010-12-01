
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class KBreadcrumbNavigationFactory;

class MainWindow : public QWidget
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);

private slots:
  void doSelects();

private:
  KBreadcrumbNavigationFactory *m_bnf;
  QStandardItemModel *m_model;

};

#endif

