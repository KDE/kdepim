/*
  Copyright (c) 2009-2015 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FOLDERCOLLECTIONMONITOR_H
#define MAILCOMMON_FOLDERCOLLECTIONMONITOR_H

#include "mailcommon_export.h"

#include <KSharedConfig>
#include <KIO/Job>

#include <QModelIndex>
#include <QObject>

class QAbstractItemModel;

namespace Akonadi
{
class ChangeRecorder;
class Collection;
class Session;
}

namespace MailCommon
{
class FolderCollectionMonitorPrivate;
class MAILCOMMON_EXPORT FolderCollectionMonitor : public QObject
{
    Q_OBJECT

public:
    explicit FolderCollectionMonitor(Akonadi::Session *session, QObject *parent = Q_NULLPTR);
    ~FolderCollectionMonitor();

    Akonadi::ChangeRecorder *monitor() const;
    void expireAllFolders(bool immediate, QAbstractItemModel *collectionModel);
    void expunge(const Akonadi::Collection &, bool sync = false);

private Q_SLOTS:
    void slotDeleteJob(KJob *job);

protected:
    void expireAllCollection(const QAbstractItemModel *model, bool immediate,
                             const QModelIndex &parentIndex = QModelIndex());

private:
    FolderCollectionMonitorPrivate *const d;
};

}

#endif
