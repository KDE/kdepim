// -*- c-basic-offset: 2 -*-

/***************************************************************************
                     kngrouplistwidget.h - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNGROUPLISTWIDGET_H
#define KNGROUPLISTWIDGET_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstrlist.h>
#include <qpushbutton.h>
#include <qtabbar.h>
#include "knnntpaccount.h"

class QListView;
class QListViewItem;

class KNGroupListWidget : public QWidget  {

  Q_OBJECT

public:
		
  KNGroupListWidget(KNNntpAccount *a, QWidget *parent=0);
  ~KNGroupListWidget();

  QString currentText() const;
  QStrList* activeList() const  { return subList; }

  void newList() { loadList();  }

  KNNntpAccount* account() const { return a_ccount; }

protected:
  QListView *m_list;
  QLineEdit *filterEdit;
  KNNntpAccount *a_ccount;		
  QStrList *allList, *subList;

protected slots:
  void loadList();
  void itemExpand(QListViewItem*);
  void itemSelected(QListViewItem*);
  void slotFilter(const QString& text);

signals:
  void itemSelected(const QString& text);
};

#endif









