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

#ifndef TRIP_H
#define TRIP_H

#include <QPersistentModelIndex>

#include <AkonadiCore/collection.h>
#include "itemselection.h"
#include <QItemSelectionModel>

class QSortFilterProxyModel;
class TripComponentFactory;
class TripModel;

namespace Akonadi
{
class ChangeRecorder;
}

class Trip : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString eventName READ eventName NOTIFY eventNameChanged)
    Q_PROPERTY(QString eventDescription READ eventDescription NOTIFY eventDescriptionChanged)
    Q_PROPERTY(QObject *mailModel READ scriptableMailModel CONSTANT)
    Q_PROPERTY(QObject *todoModel READ scriptableTodoModel CONSTANT)
    Q_PROPERTY(QObject *notesModel READ scriptableNotesModel CONSTANT)
    Q_PROPERTY(QObject *mailSelection READ mailSelection CONSTANT)
    Q_PROPERTY(QObject *todoSelection READ todoSelection CONSTANT)
    Q_PROPERTY(QObject *notesSelection READ notesSelection CONSTANT)
    Q_PROPERTY(QObject *itemSelection READ itemSelection CONSTANT)
    Q_PROPERTY(qint64 id READ id CONSTANT)
    Q_ENUMS(TripContentRoles)
public:
    enum TripContentRoles {
        MailCollectionRole,
        TodoCollectionRole,
        NotesCollectionRole
    };
    Trip(const QPersistentModelIndex &index, Akonadi::ChangeRecorder *changeRecorder, TripComponentFactory *factory, QObject *parent = 0);

    QString eventName() const;
    QString eventDescription() const;

    QAbstractItemModel *mailModel() const;
    QAbstractItemModel *todoModel() const;
    QAbstractItemModel *notesModel() const;
    QObject *scriptableMailModel() const;
    QObject *scriptableTodoModel() const;
    QObject *scriptableNotesModel() const;

    void setCollection(int role, const Akonadi::Collection &collection);
    Akonadi::Collection collection(int role);

    QModelIndex index() const;

    qint64 id() const;

    QObject *itemSelection() const;
    QObject *mailSelection() const;
    QObject *todoSelection() const;
    QObject *notesSelection() const;

signals:
    void eventNameChanged();
    void eventDescriptionChanged();
    void monitoredCollectionsChanged();

private slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void layoutChanged();
    void modelReset();
    void rowsRemoved(const QModelIndex &parent, int start, int end);

    void setComponentFilter(const QString &filter);

private:
    void setEventName(const QString &name);
    void setEventDescription(const QString &description);
    void updateEvent();

private:
    const QPersistentModelIndex m_index;
    Akonadi::ChangeRecorder *m_changeRecorder;

    Akonadi::ChangeRecorder *m_mailChangeRecorder;
    Akonadi::ChangeRecorder *m_todoChangeRecorder;
    Akonadi::ChangeRecorder *m_notesChangeRecorder;
    QSortFilterProxyModel *m_mailModel;
    QSortFilterProxyModel *m_todoModel;
    QSortFilterProxyModel *m_notesModel;

    QString m_eventName;
    QString m_eventDescription;
    ItemSelection *m_itemSelection;
    QObject *m_mailSelection;
    QObject *m_todoSelection;
    QObject *m_notesSelection;
};

#endif
