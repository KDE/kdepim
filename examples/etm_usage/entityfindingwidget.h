/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// READ THE README FILE

#ifndef ENTITYFINDINGWIDGET_H
#define ENTITYFINDINGWIDGET_H

#include <QWidget>

class QListView;

#include "entitytreewidget.h"

class FindingETW : public EntityTreeWidget
{
  Q_OBJECT
public:
  FindingETW(QWidget* parent = 0);

  virtual void connectTreeToModel(QTreeView* tree, Akonadi::EntityTreeModel* model);

signals:
  void initialized();

};

class EntityFindingWidget : public QWidget
{
  Q_OBJECT
public:
  EntityFindingWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:

  void initWidget();

  void findCollection();
  void findItem();

private:
  FindingETW *m_etw;
  QListView *m_selectionView;
  QLineEdit *m_collectionIdInput;
  QLineEdit *m_itemIdInput;

};

#endif
