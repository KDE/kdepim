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
#include "followupremindernoanswerdialog.h"
#include "followupreminderjob.h"
#include "followupreminderfinishtaskjob.h"

#include <KConfigGroup>
#include <KConfig>
#include <KSharedConfig>
#include <knotification.h>
#include <KLocalizedString>
#include <KIconLoader>
#include <KGlobal>
using namespace FollowUpReminder;

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
    bool noAnswerInfoFound = false;
    for (int i = 0 ; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));

        FollowUpReminderInfo *info = new FollowUpReminderInfo(group);
        if (info->isValid()) {
            mFollowUpReminderInfoList.append(info);
            if( info->followUpReminderDate().date() > currentDate) {
                noAnswerInfoFound = true;
                //TODO
            }
        } else {
            delete info;
        }
    }
    if (noAnswerInfoFound) {
        if (!mNoAnswerDialog.data()) {
            mNoAnswerDialog = new FollowUpReminderNoAnswerDialog;
        }
        mNoAnswerDialog->show();
    }
}

void FollowUpReminderManager::checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col)
{
    FollowUpReminderJob *job = new FollowUpReminderJob(this);
    connect(job, SIGNAL(finished(QString)), SLOT(slotCheckFollowUpFinished(QString)));
    job->setItem(item);
    job->start();
}

void FollowUpReminderManager::slotCheckFollowUpFinished(const QString &messageId)
{
    Q_FOREACH(FollowUpReminderInfo* info, mFollowUpReminderInfoList) {
        if (info->messageId() == messageId) {
            answerReceived(info->to());
            //Remove info in list and settings
            //Close task
            FollowUpReminderFinishTaskJob *job = new FollowUpReminderFinishTaskJob(this);
            //TODO
            job->start();
            break;
        }
    }
}

void FollowUpReminderManager::answerReceived(const QString &from)
{
    const QPixmap pixmap = QIcon::fromTheme( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );
#if 0 //QT5
    KNotification::event( QLatin1String("mailreceived"),
                          i18n("Answer from %1 received", from),
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());

#endif
}

QString FollowUpReminderManager::printDebugInfo()
{
    QString infoStr;
    Q_FOREACH (FollowUpReminder::FollowUpReminderInfo *info, mFollowUpReminderInfoList) {
        if (!infoStr.isEmpty())
            infoStr += QLatin1Char('\n');
        infoStr += infoToStr(info);
    }
    return infoStr;
}

QString FollowUpReminderManager::infoToStr(FollowUpReminder::FollowUpReminderInfo *info)
{
    QString infoStr;
    //TODO
    return infoStr;
}

