/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "followupremindermanager.h"
#include "followupreminderinfo.h"

#include <KConfigGroup>
#include <KConfig>
#include <KSharedConfig>

FollowUpReminderManager::FollowUpReminderManager(QObject *parent)
    : QObject(parent)
{
    mConfig = KSharedConfig::openConfig();
}

FollowUpReminderManager::~FollowUpReminderManager()
{
    qDeleteAll(mFollowUpReminderInfoList);
    mFollowUpReminderInfoList.clear();
}

void FollowUpReminderManager::load()
{
    const QStringList itemList = mConfig->groupList().filter( QRegExp( QLatin1String("FollowupReminderItem \\d+") ) );
    const int numberOfItems = itemList.count();
    const QDate currentDate = QDate::currentDate();
    for (int i = 0 ; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));

        FollowUpReminderInfo *info = new FollowUpReminderInfo(group);
        if (info->isValid()) {
            mFollowUpReminderInfoList.append(info);
            if( info->followUpReminderDate().date() > currentDate) {
                //TODO
            }
        } else {
            delete info;
        }
    }
    //if (!mNoAnswerDialog.data()) {

    //}
}

void FollowUpReminderManager::checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col)
{
    //TODO
}

#include "followupremindermanager.moc"


