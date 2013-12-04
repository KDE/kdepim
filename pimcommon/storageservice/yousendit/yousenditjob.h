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

#ifndef YOUSENDITJOB_H
#define YOUSENDITJOB_H

#include <QObject>
#include "storageservice/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon {
class YouSendItJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit YouSendItJob(QObject *parent=0);
    ~YouSendItJob();

    void requestTokenAccess();
    void uploadFile(const QString &filename);
    void listFolder();
    void accountInfo();
    void createFolder(const QString &filename);
    void shareLink(const QString &root, const QString &path);

private slots:
    void slotSendDataFinished(QNetworkReply *reply);    
};
}

#endif // YOUSENDITJOB_H
