/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
#include "followupreminderutil.h"
#include "followupremindernoanswerdialog.h"
#include "jobs/followupreminderjob.h"
#include "jobs/followupreminderfinishtaskjob.h"
#include <Akonadi/KMime/SpecialMailCollections>

#include <QIcon>

#include <KConfigGroup>
#include <KConfig>
#include <KSharedConfig>
#include <knotification.h>
#include <KLocalizedString>
#include <KIconLoader>
using namespace FollowUpReminder;

FollowUpReminderManager::FollowUpReminderManager(QObject *parent)
    : QObject(parent),
      mInitialize(false)
{
    mConfig = KSharedConfig::openConfig();
}

FollowUpReminderManager::~FollowUpReminderManager()
{
    qDeleteAll(mFollowUpReminderInfoList);
    mFollowUpReminderInfoList.clear();
}

void FollowUpReminderManager::load(bool forceReloadConfig)
{
    if (forceReloadConfig) {
        mConfig->reparseConfiguration();
    }
    const QStringList itemList = mConfig->groupList().filter(QRegExp(QStringLiteral("FollowupReminderItem \\d+")));
    const int numberOfItems = itemList.count();
    QList<FollowUpReminder::FollowUpReminderInfo *> noAnswerList;
    for (int i = 0 ; i < numberOfItems; ++i) {

        KConfigGroup group = mConfig->group(itemList.at(i));

        FollowUpReminderInfo *info = new FollowUpReminderInfo(group);
        if (info->isValid()) {
            if (!info->answerWasReceived()) {
                mFollowUpReminderInfoList.append(info);
                if (!mInitialize) {
                    FollowUpReminderInfo *noAnswerInfo = new FollowUpReminderInfo(*info);
                    noAnswerList.append(noAnswerInfo);
                }
            }
        } else {
            delete info;
        }
    }
    if (!noAnswerList.isEmpty()) {
        mInitialize = true;
        if (!mNoAnswerDialog.data()) {
            mNoAnswerDialog = new FollowUpReminderNoAnswerDialog;
        }
        mNoAnswerDialog->setInfo(noAnswerList);
        mNoAnswerDialog->show();
    }
}

void FollowUpReminderManager::checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col)
{
    if (mFollowUpReminderInfoList.isEmpty()) {
        return;
    }

    //If we move to trash directly => exclude it.
    Akonadi::SpecialMailCollections::Type type = Akonadi::SpecialMailCollections::self()->specialCollectionType(col);
    if (type == Akonadi::SpecialMailCollections::Trash) {
        return;
    }

    //Exclude outbox too
    if (type == Akonadi::SpecialMailCollections::Outbox) {
        return;
    }
    if (type == Akonadi::SpecialMailCollections::Drafts) {
        return;
    }
    if (type == Akonadi::SpecialMailCollections::Templates) {
        return;
    }
    if (type == Akonadi::SpecialMailCollections::SentMail) {
        return;
    }

    FollowUpReminderJob *job = new FollowUpReminderJob(this);
    connect(job, &FollowUpReminderJob::finished, this, &FollowUpReminderManager::slotCheckFollowUpFinished);
    job->setItem(item);
    job->start();
}

void FollowUpReminderManager::slotCheckFollowUpFinished(const QString &messageId, Akonadi::Item::Id id)
{
    Q_FOREACH (FollowUpReminderInfo *info, mFollowUpReminderInfoList) {
        if (info->messageId() == messageId) {
            info->setAnswerMessageItemId(id);
            info->setAnswerWasReceived(true);
            answerReceived(info->to());
            if (info->todoId() != -1) {
                FollowUpReminderFinishTaskJob *job = new FollowUpReminderFinishTaskJob(info->todoId(), this);
                connect(job, &FollowUpReminderFinishTaskJob::finishTaskDone, this, &FollowUpReminderManager::slotFinishTaskDone);
                connect(job, &FollowUpReminderFinishTaskJob::finishTaskFailed, this, &FollowUpReminderManager::slotFinishTaskFailed);
                job->start();
            }
            //Save item
            FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(FollowUpReminder::FollowUpReminderUtil::defaultConfig(), info, true);
            break;
        }
    }
}

void FollowUpReminderManager::slotFinishTaskDone()
{
    //TODO
}

void FollowUpReminderManager::slotFinishTaskFailed()
{
    //TODO
}

void FollowUpReminderManager::answerReceived(const QString &from)
{
    const QPixmap pixmap = QIcon::fromTheme(QStringLiteral("kmail")).pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    KNotification::event(QStringLiteral("mailreceived"),
                         i18n("Answer from %1 received", from),
                         pixmap,
                         0,
                         KNotification::CloseOnTimeout,
                         QStringLiteral("akonadi_followupreminder_agent"));

}

QString FollowUpReminderManager::printDebugInfo()
{
    QString infoStr;
    if (mFollowUpReminderInfoList.isEmpty()) {
        infoStr = QLatin1String("No mail");
    } else {
        Q_FOREACH (FollowUpReminder::FollowUpReminderInfo *info, mFollowUpReminderInfoList) {
            if (!infoStr.isEmpty()) {
                infoStr += QLatin1Char('\n');
            }
            infoStr += infoToStr(info);
        }
    }
    return infoStr;
}

QString FollowUpReminderManager::infoToStr(FollowUpReminder::FollowUpReminderInfo *info)
{
    QString infoStr;
    infoStr = QLatin1String("****************************************");
    infoStr += QString::fromLatin1("Akonadi Item id :%1\n").arg(info->originalMessageItemId());
    infoStr += QString::fromLatin1("MessageId :%1\n").arg(info->messageId());
    infoStr += QString::fromLatin1("Subject :%1\n").arg(info->subject());
    infoStr += QString::fromLatin1("To :%1\n").arg(info->to());
    infoStr += QString::fromLatin1("Dead Line :%1\n").arg(info->followUpReminderDate().toString());
    infoStr += QString::fromLatin1("Answer received :%1\n").arg(info->answerWasReceived() ? QLatin1String("true") : QLatin1String("false"));
    infoStr += QLatin1String("****************************************\n");
    return infoStr;
}

