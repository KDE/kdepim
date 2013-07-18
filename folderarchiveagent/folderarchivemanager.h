/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef FOLDERARCHIVEMANAGER_H
#define FOLDERARCHIVEMANAGER_H

#include <QObject>
#include <QQueue>
namespace Akonadi {
class AgentInstance;
}

class FolderArchiveAccountInfo;
class FolderArchiveKernel;
class FolderArchiveAgentJob;
class FolderArchiveManager : public QObject
{
    Q_OBJECT
public:
    explicit FolderArchiveManager(QObject *parent=0);
    ~FolderArchiveManager();

    void load();
    void setArchiveItems(const QList<qint64> &itemIds, const QString &instanceName);

    void moveFailed(const QString &msg);
    void moveDone(const QString &msg);

private Q_SLOTS:
    void slotInstanceRemoved(const Akonadi::AgentInstance &instance);

private:
    FolderArchiveAccountInfo *infoFromInstanceName(const QString &instanceName) const;
    void nextJob();
    void removeInfo(const QString &instanceName);
    QQueue<FolderArchiveAgentJob*> mJobQueue;
    FolderArchiveAgentJob *mCurrentJob;
    QList<FolderArchiveAccountInfo*> mListAccountInfo;
    FolderArchiveKernel *mFolderArchivelKernel;
};

#endif // FOLDERARCHIVEMANAGER_H
