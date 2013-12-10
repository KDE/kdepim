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

#ifndef BoxJob_H
#define BoxJob_H

#include <QObject>
#include <QPointer>
#include "storageservice/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon {
class StorageAuthViewDialog;

class BoxJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit BoxJob(QObject *parent=0);
    ~BoxJob();

    void requestTokenAccess();
    void uploadFile(const QString &filename);
    void listFolder();
    void accountInfo();
    void createFolder(const QString &filename);
    void shareLink(const QString &root, const QString &path);

Q_SIGNALS:
    void authorizationDone(const QString &refreshToken);

private slots:
    void slotSendDataFinished(QNetworkReply *reply);
    void slotRedirect(const QUrl &url);

private:
    void parseUploadFiles(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseListFolder(const QString &data);
    void parseRedirectUrl(const QUrl &url);
    void getTokenAccess(const QString &authorizeCode);
    QString mClientId;
    QString mClientSecret;
    QString mRedirectUri;
    QUrl mAuthUrl;
    QPointer<PimCommon::StorageAuthViewDialog> mAuthDialog;
};
}

#endif // BoxJob_H
