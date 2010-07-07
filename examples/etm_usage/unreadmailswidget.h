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

#ifndef UNREADMAILSWIDGET_H
#define UNREADMAILSWIDGET_H

#include <QWidget>

#include <akonadi_next/krecursivefilterproxymodel.h>


class QTreeView;
class EntityTreeWidget;

class UnreadMailsTree : public Future::KRecursiveFilterProxyModel
{
  Q_OBJECT
public:
  UnreadMailsTree(QObject* parent = 0);

protected:
  /* reimp */ bool acceptRow(int sourceRow, const QModelIndex& sourceParent) const;

};

class UnreadMailsWidget : public QWidget
{
  Q_OBJECT
public:
  UnreadMailsWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

private:
  QTreeView *m_itemView;
  EntityTreeWidget *m_etw;
};

#endif
