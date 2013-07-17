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

#include "folderarchivemanager.h"
#include "folderarchiveaccountinfo.h"
#include "folderarchivekernel.h"

#include <mailcommon/kernel/mailkernel.h>

#include <Akonadi/AgentManager>

#include <KSharedConfig>
#include <KGlobal>

FolderArchiveManager::FolderArchiveManager(QObject *parent)
    : QObject(parent)
{
    mFolderArchivelKernel = new FolderArchiveKernel( this );
    CommonKernel->registerKernelIf( mFolderArchivelKernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( mFolderArchivelKernel ); //SettingsIf is used in FolderTreeWidget

    connect( Akonadi::AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)),
             this, SLOT(slotInstanceRemoved(Akonadi::AgentInstance)) );

}

FolderArchiveManager::~FolderArchiveManager()
{
    qDeleteAll(mListAccountInfo);
    mListAccountInfo.clear();
}

void FolderArchiveManager::setArchiveItems(const QList<qint64> &itemIds, const QString &instanceName)
{

}

void FolderArchiveManager::slotInstanceRemoved(const Akonadi::AgentInstance &instance)
{
    const QString instanceName = instance.name();
    Q_FOREACH (FolderArchiveAccountInfo *info, mListAccountInfo) {
        if (info->instanceName() == instanceName) {
            mListAccountInfo.removeAll(info);
            removeInfo(instanceName);
            break;
        }
    }
}

void FolderArchiveManager::removeInfo(const QString &instanceName)
{
    KConfigGroup group = KGlobal::config()->group(QLatin1String("FolderArchiveAccount ") + instanceName);
    group.deleteGroup();
    KGlobal::config()->sync();
}

void FolderArchiveManager::load()
{
    qDeleteAll(mListAccountInfo);
    mListAccountInfo.clear();

    const QStringList accountList = KGlobal::config()->groupList().filter( QRegExp( QLatin1String("FolderArchiveAccount ") ) );
    Q_FOREACH (const QString &account, accountList) {
        KConfigGroup group = KGlobal::config()->group(account);
        FolderArchiveAccountInfo *info = new FolderArchiveAccountInfo(group);
        if (info->enabled()) {
            //TODO verify isValid();
            mListAccountInfo.append(info);
        } else {
            delete info;
        }
    }

    //TODO
}

#include "folderarchivemanager.moc"
