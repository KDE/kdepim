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

#ifndef HUBICJOB_H
#define HUBICJOB_H

#include <QObject>
#include "storageservice/storageserviceabstractjob.h"
#include <QPointer>
class QNetworkReply;
namespace PimCommon {
class StorageAuthViewDialog;
class HubicJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit HubicJob(QObject *parent=0);
    ~HubicJob();

    void requestTokenAccess();
    void uploadFile(const QString &filename);
    void listFolder();
    void accountInfo();
    void createFolder(const QString &filename);
    void shareLink(const QString &root, const QString &path);

Q_SIGNALS:
    void authorizationDone(const QString &refreshToken);
    void authorizationFailed();

private slots:
    void slotSendDataFinished(QNetworkReply *reply);
    void slotRedirect(const QUrl &url);

private:
    void parseRedirectUrl(const QUrl &url);
    void parseAccessToken(const QString &data);
    void getTokenAccess(const QString &authorizeCode);
    void refreshToken();
    QUrl mAuthUrl;
    QString mClientId;
    QString mClientSecret;
    QString mRedirectUri;
    QString mRefreshToken;
    QString mToken;
    qint64 mExpireInTime;
    QPointer<PimCommon::StorageAuthViewDialog> mAuthDialog;
};
}

#endif // HUBICJOB_H
