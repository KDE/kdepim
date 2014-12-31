/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef NOTESMANAGER_H
#define NOTESMANAGER_H

#include <QObject>

#include "notesagentalarmdialog.h"

#include <Item>
#include <QPointer>
class QTcpServer;
class QTimer;
namespace NoteShared
{
class NotesChangeRecorder;
class NotesAkonadiTreeModel;
}
class QModelIndex;
class NotesManager : public QObject
{
    Q_OBJECT
public:
    explicit NotesManager(QObject *parent = Q_NULLPTR);
    ~NotesManager();

    void stopAll();
    void updateNetworkListener();

public Q_SLOTS:
    void load();

private Q_SLOTS:
    void slotAcceptConnection();
    void slotNewNote(const QString &name, const QString &text);
    void slotCheckAlarm();

    void slotItemRemoved(const Akonadi::Item &item);
    void slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &set);
    void slotRowInserted(const QModelIndex &parent, int start, int end);
private:
    void clear();
    Akonadi::Item::List mListItem;
    QTcpServer *mListener;
    QTimer *mCheckAlarm;
    NoteShared::NotesChangeRecorder *mNoteRecorder;
    NoteShared::NotesAkonadiTreeModel *mNoteTreeModel;
    QPointer<NotesAgentAlarmDialog> mAlarmDialog;
};

#endif // NOTESMANAGER_H
