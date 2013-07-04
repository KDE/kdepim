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

#include "sendlatermanager.h"
#include "sendlaterinfo.h"
#include "sendlaterutil.h"
#include "sendlaterjob.h"

#include "messagecomposer/sender/akonadisender.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KMessageBox>
#include <KLocale>

#include <QStringList>
#include <QTimer>

SendLaterManager::SendLaterManager(QObject *parent)
    : QObject(parent),
      mCurrentInfo(0),
      mCurrentJob(0),
      mSender(new MessageComposer::AkonadiSender)
{
    mConfig = KGlobal::config();
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(slotCreateJob()));
}

SendLaterManager::~SendLaterManager()
{
    stopAll();
    delete mSender;
}

void SendLaterManager::stopAll()
{
    stopTimer();
    qDeleteAll(mListSendLaterInfo);
    mListSendLaterInfo.clear();
    mCurrentJob = 0;
}

void SendLaterManager::load(bool forcereload)
{
    stopAll();
    if (forcereload)
        mConfig->reparseConfiguration();

    const QStringList itemList = mConfig->groupList().filter( QRegExp( QLatin1String("SendLaterItem \\d+") ) );
    const int numberOfItems = itemList.count();
    for (int i = 0 ; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));
        SendLater::SendLaterInfo *info = new SendLater::SendLaterInfo(group);        
        mListSendLaterInfo.append(info);
    }
    createSendInfoList();
}

void SendLaterManager::createSendInfoList()
{
    mCurrentInfo = 0;
    qSort(mListSendLaterInfo.begin(), mListSendLaterInfo.end(), SendLater::SendLaterUtil::compareSendLaterInfo);

    //Look at QQueue
    if (mSendLaterQueue.isEmpty()) {
        if (!mListSendLaterInfo.isEmpty()) {
            mCurrentInfo = mListSendLaterInfo.first();
            const QDateTime now = QDateTime::currentDateTime();
            const int seconds = now.secsTo(mCurrentInfo->dateTime());
            if (seconds > 0) {
                //qDebug()<<" seconds"<<seconds;
                mTimer->start(seconds*1000);
            } else {
                //Create job when seconds <0
                slotCreateJob();
            }
        } else {
            qDebug()<<" list is empty";
        }
    } else {
        SendLater::SendLaterInfo *info = searchInfo(mSendLaterQueue.dequeue());
        if (info) {
            mCurrentInfo = info;
            slotCreateJob();
        } else { //If removed.
            createSendInfoList();
        }
    }
}

void SendLaterManager::stopTimer()
{
    if (mTimer->isActive())
        mTimer->stop();
}

SendLater::SendLaterInfo *SendLaterManager::searchInfo(Akonadi::Item::Id id)
{
    Q_FOREACH(SendLater::SendLaterInfo *info, mListSendLaterInfo) {
        if (info->itemId() == id) {
            return info;
        }
    }
    return 0;
}

void SendLaterManager::sendNow(Akonadi::Item::Id id)
{
    if (!mCurrentJob) {
        SendLater::SendLaterInfo *info = searchInfo(id);
        if (info) {
            mCurrentInfo = info;
            slotCreateJob();
        } else {
            qDebug()<<" can't find info about current id: "<<id;
            itemRemoved(id);
        }
    } else {
        //Add to QQueue
        mSendLaterQueue.enqueue(id);
    }
}

void SendLaterManager::slotCreateJob()
{
    mCurrentJob = new SendLaterJob(this, mCurrentInfo);
    mCurrentJob->start();
}

void SendLaterManager::itemRemoved(Akonadi::Item::Id id)
{
    if (mConfig->hasGroup(QString::fromLatin1("SendLaterItem %1").arg(id))) {
        removeInfo(id);
        mConfig->reparseConfiguration();
        Q_EMIT needUpdateConfigDialogBox();
    }
}

void SendLaterManager::removeInfo(Akonadi::Item::Id id)
{
    KConfigGroup group = mConfig->group(QString::fromLatin1("SendLaterItem %1").arg(id));
    group.deleteGroup();
    group.sync();
}

void SendLaterManager::sendError(SendLater::SendLaterInfo *info, ErrorType type)
{
    if (info) {
        if (type == ItemNotFound) {
            //Don't try to resend it. Remove it.
            mListSendLaterInfo.removeAll(mCurrentInfo);
            removeInfo(info->itemId());
        } else {
            if (KMessageBox::Yes == KMessageBox::questionYesNo(0, i18n("An error was found. Do you want to resend it?"), i18n("Error found"))) {
                //TODO 4.12: allow to remove it even if it's recurrent (need new i18n)
                if (!info->isRecurrence()) {
                    mListSendLaterInfo.removeAll(mCurrentInfo);
                    removeInfo(info->itemId());
                }
            } else {
                mListSendLaterInfo.removeAll(mCurrentInfo);
                removeInfo(info->itemId());
            }
        }
    }
    recreateSendList();
}

void SendLaterManager::recreateSendList()
{
    mCurrentJob = 0;
    Q_EMIT needUpdateConfigDialogBox();
    QTimer::singleShot(1000*60, this, SLOT(createSendInfoList()));
}

void SendLaterManager::sendDone(SendLater::SendLaterInfo *info)
{
    if (info) {
        if (info->isRecurrence()) {
            SendLater::SendLaterUtil::changeRecurrentDate(info);
        } else {
            mListSendLaterInfo.removeAll(mCurrentInfo);
            removeInfo(info->itemId());
        }
    }
    recreateSendList();
}

void SendLaterManager::printDebugInfo()
{
    Q_FOREACH (SendLater::SendLaterInfo *info, mListSendLaterInfo) {
        kDebug() <<" recusive "<<info->isRecurrence() <<
                   " id :"<<info->itemId()<<
                   " date :"<<info->dateTime().toString()<<
                   " last saved date"<<info->lastDateTimeSend().toString();
    }
}

MessageComposer::AkonadiSender *SendLaterManager::sender() const
{
    return mSender;
}

#include "sendlatermanager.moc"
