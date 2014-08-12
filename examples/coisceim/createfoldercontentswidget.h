/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef CREATEFOLDERCONTENTSWIDGET_H
#define CREATEFOLDERCONTENTSWIDGET_H

#include <QStackedWidget>
#include <AkonadiCore/Collection>

class FolderContentsWidget;
class QAbstractItemModel;

class Trip;

class CreateFolderContentsWidget : public QStackedWidget
{
  Q_OBJECT
public:
  explicit CreateFolderContentsWidget(Trip *trip, int role, const QString &type, QWidget* parent = 0);

  void clear();

private slots:
  void collectionSelected();

private:
  FolderContentsWidget *m_widget;
  Trip *m_trip;
  int m_role;
};

#endif
