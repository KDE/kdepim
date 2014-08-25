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

#ifndef STACKEDWIDGETVIEW_H
#define STACKEDWIDGETVIEW_H

#include <QStackedWidget>

class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;

class StackedWidgetView : public QStackedWidget
{
    Q_OBJECT
public:
    explicit StackedWidgetView(int widgetRole, QWidget *parent = 0);

    void setModel(QAbstractItemModel *model);
    void setSelectionModel(QItemSelectionModel *selectionModel);

private slots:
    void insertRows(const QModelIndex &parent, int start, int end);
    void removeRows(const QModelIndex &parent, int start, int end);
    void refill();

    void updateCurrentWidget();

private:
    QAbstractItemModel *m_model;
    QItemSelectionModel *m_selectionModel;
    const int m_widgetRole;
};

#endif
