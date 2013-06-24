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
      mCurrentJob(0)
{
    mConfig = KGlobal::config();
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(slotCreateJob()));
}

SendLaterManager::~SendLaterManager()
{
    stopTimer();
    qDeleteAll(mListSendLaterInfo);
    delete mCurrentJob;
}

void SendLaterManager::load()
{
    stopTimer();
    qDeleteAll(mListSendLaterInfo);
    mListSendLaterInfo.clear();

    const QStringList itemList = mConfig->groupList().filter( QRegExp( QLatin1String("SendLaterItem \\d+") ) );
    const int numberOfItems = itemList.count();
    for (int i = 0 ; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));
        SendLaterInfo *info = new SendLaterInfo(group);        
        mListSendLaterInfo.append(info);
    }
    createSendInfoList();
}

void SendLaterManager::createSendInfoList()
{
    mCurrentInfo = 0;
    qSort(mListSendLaterInfo.begin(), mListSendLaterInfo.end(), SendLaterUtil::compareSendLaterInfo);
    if (!mListSendLaterInfo.isEmpty()) {
        mCurrentInfo = mListSendLaterInfo.first();
        const QDateTime now = QDateTime::currentDateTime();
        const int seconds = now.secsTo(mCurrentInfo->dateTime());
        if (seconds > 0) {
            mTimer->start(seconds*1000);
        } else {
            //Create job when seconds <0
            slotCreateJob();
        }
    }
}

void SendLaterManager::stopTimer()
{
    if (mTimer->isActive())
        mTimer->stop();
}

void SendLaterManager::slotCreateJob()
{
    mCurrentJob = new SendLaterJob(this, mCurrentInfo);
    mCurrentJob->start();
}

void SendLaterManager::removeInfo(Akonadi::Item::Id id)
{
    KConfigGroup group = mConfig->group(QString::fromLatin1("SendLaterItem %1").arg(id));
    group.deleteGroup();
    group.sync();
}

void SendLaterManager::sendError(SendLaterInfo *info, ErrorType type)
{
    if (info) {
        if (type == ItemNotFound) {
            //Don't try to resend it. Remove it.
            mListSendLaterInfo.removeAll(mCurrentInfo);
            removeInfo(info->itemId());
        } else {
            if (KMessageBox::Yes == KMessageBox::questionYesNo(0, i18n("An error was found. Do you want to resend it?"), i18n("Error found"))) {
                if (!info->isRecursive()) {
                    mListSendLaterInfo.removeAll(mCurrentInfo);
                    removeInfo(info->itemId());
                }
            } else {
                mListSendLaterInfo.removeAll(mCurrentInfo);
                removeInfo(info->itemId());
            }
        }
    }
    delete mCurrentJob;
    createSendInfoList();
}

void SendLaterManager::sendDone(SendLaterInfo *info)
{
    if (info) {
        if (!info->isRecursive()) {
            mListSendLaterInfo.removeAll(mCurrentInfo);
            removeInfo(info->itemId());
        }
    }
    delete mCurrentJob;
    createSendInfoList();
}

void SendLaterManager::printDebugInfo()
{
    //TODO
}

#include "sendlatermanager.moc"
