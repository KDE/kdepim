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

#ifndef SENDLATERMANAGER_H
#define SENDLATERMANAGER_H

#include <QObject>

#include <Akonadi/Item>

#include <KSharedConfig>


class SendLaterInfo;
class QTimer;
class SendLaterJob;
class SendLaterManager : public QObject
{
    Q_OBJECT
public:
    enum ErrorType {
        ItemNotFound = 0,
        TooManyItemFound = 1,
        CanNotFetchItem = 2
    };

    explicit SendLaterManager(QObject *parent);
    ~SendLaterManager();

    void sendDone(SendLaterInfo *info);
    void sendError(SendLaterInfo *info, ErrorType type);
    void printDebugInfo();

Q_SIGNALS:
    void needUpdateConfigDialogBox();

public Q_SLOTS:
    void load();

private Q_SLOTS:
    void slotCreateJob();

private:
    void createSendInfoList();
    void stopTimer();
    void removeInfo(Akonadi::Item::Id id);
    KSharedConfig::Ptr mConfig;
    QList<SendLaterInfo *> mListSendLaterInfo;
    SendLaterInfo *mCurrentInfo;
    SendLaterJob *mCurrentJob;
    QTimer *mTimer;
};

#endif // SENDLATERMANAGER_H
