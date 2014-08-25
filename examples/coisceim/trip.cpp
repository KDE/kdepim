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

#include "trip.h"

#include <QSortFilterProxyModel>

#include <AkonadiCore/ChangeRecorder>

#include <KCalCore/Incidence>

#include "tripcomponentfactory.h"
#include "tripmodel.h"
#include <QItemSelectionModel>
#include "itemviewerwidget.h"
#include "qmllistselectionmodel.h"
#include <QTreeView>

using namespace Akonadi;

Trip::Trip(const QPersistentModelIndex &index, Akonadi::ChangeRecorder *changeRecorder, TripComponentFactory *factory, QObject *parent)
    : QObject(parent), m_index(index), m_changeRecorder(changeRecorder)
{
    QAbstractItemModel *model = const_cast<QAbstractItemModel *>(m_index.model());
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(dataChanged(QModelIndex, QModelIndex)));
    connect(model, SIGNAL(modelReset()), SLOT(modelReset()));
    connect(model, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(rowsRemoved(QModelIndex, int, int)));

    m_mailChangeRecorder = factory->createMailChangeRecorder(this);
    m_todoChangeRecorder = factory->createTodoChangeRecorder(this);
    m_notesChangeRecorder = factory->createNotesChangeRecorder(this);

    m_mailChangeRecorder->setSession(m_changeRecorder->session());
    m_todoChangeRecorder->setSession(m_changeRecorder->session());
    m_notesChangeRecorder->setSession(m_changeRecorder->session());

    m_mailModel = new QSortFilterProxyModel(this);
    m_mailModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_todoModel = new QSortFilterProxyModel(this);
    m_todoModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_notesModel = new QSortFilterProxyModel(this);
    m_notesModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_mailModel->setSourceModel(factory->createMailModel(m_mailChangeRecorder));
    m_todoModel->setSourceModel(factory->createTodoModel(m_todoChangeRecorder));
    m_notesModel->setSourceModel(factory->createNotesModel(m_notesChangeRecorder));

    QItemSelectionModel *mailItemSelection = new QItemSelectionModel(m_mailModel, this);
    QItemSelectionModel *todoItemSelection = new QItemSelectionModel(m_todoModel, this);
    QItemSelectionModel *notesItemSelection = new QItemSelectionModel(m_notesModel, this);

    m_mailSelection = new QMLListSelectionModel(mailItemSelection, this);
    m_todoSelection = new QMLListSelectionModel(todoItemSelection, this);
    m_notesSelection = new QMLListSelectionModel(notesItemSelection, this);

    m_itemSelection = new ItemSelection(mailItemSelection, todoItemSelection, notesItemSelection, this);

    updateEvent();
}

QModelIndex Trip::index() const
{
    return m_index;
}

void Trip::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.parent() == m_index.parent()) {
        if (topLeft.row() <= m_index.row() && bottomRight.row() >= m_index.row()) {
            updateEvent();
        }
    }
}

QString Trip::eventDescription() const
{
    return m_eventDescription;
}

QString Trip::eventName() const
{
    return m_eventName;
}

void Trip::layoutChanged()
{
    updateEvent();
}

void Trip::modelReset()
{
    m_eventDescription.clear();
    emit eventDescriptionChanged();
    m_eventName.clear();
    emit eventNameChanged();
}

void Trip::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    if (parent == m_index.parent() && start <= m_index.row() && end >= m_index.row()) {
        updateEvent();
    }
}

void Trip::updateEvent()
{
    Akonadi::Item item = m_index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.hasPayload<KCalCore::Incidence::Ptr>()) {
        return;
    }
    KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    setEventDescription(incidence->description());
    setEventName(incidence->summary());
}

static void updateCollection(Akonadi::ChangeRecorder *cr, const Collection &collection)
{
    foreach (const Collection &existingCollection, cr->collectionsMonitored()) {
        cr->setCollectionMonitored(existingCollection, false);
    }

    foreach (const QString &mimeType, cr->mimeTypesMonitored()) {
        cr->setMimeTypeMonitored(mimeType, false);
    }

    if (collection.isValid()) {
        cr->setCollectionMonitored(collection, true);
    }
}

void Trip::setCollection(int role, const Akonadi::Collection &collection)
{
    switch (role) {
    case MailCollectionRole:
        updateCollection(m_mailChangeRecorder, collection);
        emit monitoredCollectionsChanged();
        return;
    case TodoCollectionRole:
        updateCollection(m_todoChangeRecorder, collection);
        emit monitoredCollectionsChanged();
        return;
    case NotesCollectionRole:
        updateCollection(m_notesChangeRecorder, collection);
        emit monitoredCollectionsChanged();
        return;
    }
}

static Akonadi::Collection monitoredCollection(Akonadi::ChangeRecorder *cr)
{
    Akonadi::Collection::List list = cr->collectionsMonitored();
    if (list.isEmpty()) {
        return Akonadi::Collection(-1);
    }
    return list.first();
}

Akonadi::Collection Trip::collection(int role)
{
    switch (role) {
    case MailCollectionRole:
        return monitoredCollection(m_mailChangeRecorder);
    case TodoCollectionRole:
        return monitoredCollection(m_todoChangeRecorder);
    case NotesCollectionRole:
        return monitoredCollection(m_notesChangeRecorder);
    }
    return Akonadi::Collection();
}

QAbstractItemModel *Trip::mailModel() const
{
    return m_mailModel;
}

QAbstractItemModel *Trip::notesModel() const
{
    return m_notesModel;
}

QAbstractItemModel *Trip::todoModel() const
{
    return m_todoModel;
}

QObject *Trip::scriptableMailModel() const
{
    return m_mailModel;
}

QObject *Trip::scriptableNotesModel() const
{
    return m_notesModel;
}

QObject *Trip::scriptableTodoModel() const
{
    return m_todoModel;
}

qint64 Trip::id() const
{
    return m_index.data(EntityTreeModel::ItemIdRole).toLongLong();
}

void Trip::setEventName(const QString &name)
{
    if (m_eventName != name) {
        m_eventName = name;
        emit eventNameChanged();
    }
}

void Trip::setEventDescription(const QString &description)
{
    if (m_eventDescription != description) {
        m_eventDescription = description;
        emit eventDescriptionChanged();
    }
}

void Trip::setComponentFilter(const QString &filter)
{
    m_mailModel->setFilterRegExp(filter);
    m_todoModel->setFilterRegExp(filter);
    m_notesModel->setFilterRegExp(filter);
}

QObject *Trip::itemSelection() const
{
    return m_itemSelection;
}

QObject *Trip::mailSelection() const
{
    return m_mailSelection;
}

QObject *Trip::todoSelection() const
{
    return m_todoSelection;
}

QObject *Trip::notesSelection() const
{
    return m_notesSelection;
}
