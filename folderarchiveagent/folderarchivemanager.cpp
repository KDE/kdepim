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

#include <Akonadi/AgentManager>

#include <KSharedConfig>
#include <KGlobal>

FolderArchiveManager::FolderArchiveManager(QObject *parent)
    : QObject(parent)
{
    connect( Akonadi::AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)),
             this, SLOT(slotInstanceRemoved(Akonadi::AgentInstance)) );

}

FolderArchiveManager::~FolderArchiveManager()
{
    qDeleteAll(mListAccountInfo);
}

void FolderArchiveManager::slotInstanceRemoved(const Akonadi::AgentInstance &instance)
{
    //TODO
}

void FolderArchiveManager::load()
{
    qDeleteAll(mListAccountInfo);
    mListAccountInfo.clear();

    const QStringList accountList = KGlobal::config()->groupList().filter( QRegExp( QLatin1String("FolderArchiveAccount ") ) );
    Q_FOREACH (const QString &account, accountList) {
        KConfigGroup group = KGlobal::config()->group(account);
        FolderArchiveAccountInfo *info = new FolderArchiveAccountInfo(group);
        //TODO verify isValid();
        mListAccountInfo.append(info);
    }

    //TODO
}

#include "folderarchivemanager.moc"
