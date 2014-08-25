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

#ifndef FOLDERCONTENTSWIDGET_H
#define FOLDERCONTENTSWIDGET_H

#include <QWidget>

class QItemSelectionModel;

namespace Akonadi
{
class EntityTreeView;
}

class Trip;

class FolderContentsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FolderContentsWidget(Trip *trip, int role, const QString &type, QWidget *parent = 0, Qt::WindowFlags f = 0);

    QItemSelectionModel *selectionModel() const;

protected:
    Trip *trip() const;

private slots:
    void configure();

private:
    const QString m_type;
    Trip *m_trip;
    int m_role;
    Akonadi::EntityTreeView *m_view;
};

#endif
