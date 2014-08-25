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

#ifndef UNREADMAILSINCOLLECTIONSWIDGET_H
#define UNREADMAILSINCOLLECTIONSWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

class QItemSelectionModel;

class KCheckableProxyModel;

namespace Akonadi
{
class ChangeRecorder;
class EntityTreeModel;
}

class UnreadMailsInCollectionsProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    UnreadMailsInCollectionsProxy(QObject *parent = 0);
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

class UnreadMailsInCollectionsWidget : public QWidget
{
    Q_OBJECT
public:
    UnreadMailsInCollectionsWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~UnreadMailsInCollectionsWidget();

private slots:
    void configure();
    void saveCheckState();
    void restoreCheckState();

private:
    Akonadi::ChangeRecorder *m_changeRecorder;
    Akonadi::EntityTreeModel *m_etm;
    QItemSelectionModel *m_checkedItemModel;
    KCheckableProxyModel *m_checkableProxy;

};

#endif
