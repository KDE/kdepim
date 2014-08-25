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

#ifndef ITEMSELECTION_H
#define ITEMSELECTION_H

#include <QModelIndex>

class QItemSelectionModel;

class ItemSelection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 id READ id NOTIFY idChanged)
    Q_PROPERTY(ItemType type READ itemType NOTIFY idChanged)
    Q_PROPERTY(QString noteTitle READ noteTitle NOTIFY idChanged)
    Q_PROPERTY(QString noteContent READ noteContent NOTIFY idChanged)
    Q_ENUMS(ItemType)
public:
    enum ItemType {
        InvalidType,
        MailType,
        TodoType,
        NotesType
    };
    explicit ItemSelection(QItemSelectionModel *selModel1, QItemSelectionModel *selModel2, QItemSelectionModel *selModel3, QObject *parent = 0);

    QModelIndex index() const;

    qint64 id();
    ItemType itemType() const;

    QString noteTitle() const;
    QString noteContent() const;

    Q_INVOKABLE void clear();

signals:
    void selectionChanged(const QModelIndex &index);
    void idChanged();

private slots:
    void modelSelectionChanged();

private:
    void clearOrUpdate(QItemSelectionModel *selModel, QObject *sender_);
    void connectSignals();
    void disconnectSignals();

private:
    QItemSelectionModel *m_selModel1;
    QItemSelectionModel *m_selModel2;
    QItemSelectionModel *m_selModel3;
    QPersistentModelIndex m_index;
    qint64 m_id;
};

#endif
