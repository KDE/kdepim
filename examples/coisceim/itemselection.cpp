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

#include "itemselection.h"

#include <QItemSelectionModel>
#include <QDebug>
#include <AkonadiCore/EntityTreeModel>
#include <KCalCore/Todo>
#include <KMime/Message>
#include "note.h"

ItemSelection::ItemSelection(QItemSelectionModel *selModel1, QItemSelectionModel *selModel2, QItemSelectionModel *selModel3, QObject *parent)
    : QObject(parent), m_selModel1(selModel1), m_selModel2(selModel2), m_selModel3(selModel3), m_id(-1)
{
    connectSignals();
}

QModelIndex ItemSelection::index() const
{
    return m_index;
}

void ItemSelection::connectSignals()
{
    connect(m_selModel1, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(modelSelectionChanged()));
    connect(m_selModel2, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(modelSelectionChanged()));
    connect(m_selModel3, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(modelSelectionChanged()));
}

void ItemSelection::disconnectSignals()
{
    disconnect(m_selModel1, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(modelSelectionChanged()));
    disconnect(m_selModel2, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(modelSelectionChanged()));
    disconnect(m_selModel3, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(modelSelectionChanged()));
}

void ItemSelection::modelSelectionChanged()
{
    QObject *sender_ = sender();
    disconnectSignals();
    clearOrUpdate(m_selModel1, sender_);
    clearOrUpdate(m_selModel2, sender_);
    clearOrUpdate(m_selModel3, sender_);
    connectSignals();
}

void ItemSelection::clear()
{
    m_selModel1->clearSelection();
    m_selModel2->clearSelection();
    m_selModel3->clearSelection();
    m_id = -1;
    emit idChanged();
}

// Workaround bug in QItemSelectionModel::selectedRows
static QModelIndexList selectedRows(const QItemSelection &selection, int column = 0)
{
    QModelIndexList list;

    foreach (const QModelIndex &idx, selection.indexes())
        if (idx.column() == column) {
            list << idx;
        }

    return list;
}

void ItemSelection::clearOrUpdate(QItemSelectionModel *selModel, QObject *sender_)
{
    if (selModel != sender_) {
        selModel->clearSelection();
    } else {
        const QModelIndexList list = selectedRows(selModel->selection());
        if (list.isEmpty()) {
            return;
        }
        m_index = list.first();
        emit selectionChanged(m_index);
        m_id = m_index.data(Akonadi::EntityTreeModel::ItemIdRole).toLongLong();
        emit idChanged();
    }
}

qint64 ItemSelection::id()
{
    return m_id;
}

ItemSelection::ItemType ItemSelection::itemType() const
{
    Akonadi::Item item = m_index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (item.mimeType() == KCalCore::Todo::todoMimeType()) {
        return TodoType;
    }
    if (item.mimeType() == KMime::Message::mimeType()) {
        return MailType;
    }
    if (item.mimeType() == Akonotes::Note::mimeType()) {
        return NotesType;
    }
    return InvalidType;
}

QString ItemSelection::noteContent() const
{
    if (!m_index.isValid()) {
        return QString();
    }
    Akonadi::Item item = m_index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        return QString();
    }
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return QString();
    }

    KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    return message->mainBodyPart()->decodedText();
}

QString ItemSelection::noteTitle() const
{
    return m_index.data().toString();
}

