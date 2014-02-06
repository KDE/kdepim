/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "storageservice/job/storageserviceabstractjob.h"
namespace PimCommon {
class StorageAuthViewDialog;
class BoxJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit BoxJob(QObject *parent=0);
    ~BoxJob();

    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);

    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination);
    void listFolder(const QString &folder);
    void accountInfo();
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &root, const QString &fileId);
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);
    void refreshToken();
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination);
    void requestTokenAccess();
    void initializeToken(const QString &refreshToken, const QString &token);
    void createServiceFolder();

Q_SIGNALS:
    void authorizationDone(const QString &refreshToken, const QString &token, qint64 expireTime);

private slots:
    void slotSendDataFinished(QNetworkReply *reply);
    void slotRedirect(const QUrl &url);
private:
    void parseDeleteFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseCreateServiceFolder(const QString &data);
    void parseListFolder(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseUploadFile(const QString &data);
    void parseCopyFile(const QString &data);
    void parseCopyFolder(const QString &data);
    void parseRenameFile(const QString &data);
    void parseRenameFolder(const QString &data);
    void parseMoveFolder(const QString &data);
    void parseMoveFile(const QString &data);
    void parseShareLink(const QString &data);
    void parseDownloadFile(const QString &data);
    void getTokenAccess(const QString &authorizeCode);
    void parseRedirectUrl(const QUrl &url);
    void parseAccessToken(const QString &data);

    QString parseNameInfo(const QString &data);

    QString mServiceUrl;
    QUrl mAuthUrl;
    QString mClientId;
    QString mClientSecret;
    QString mRedirectUri;
    QString mRefreshToken;
    QString mToken;
    QString mScope;
    QString mAuthorizePath;
    QString mPathToken;
    QString mFolderInfoPath;
    QString mCurrentAccountInfoPath;
    QString mApiUrl;
    QString mFileInfoPath;
    QPointer<PimCommon::StorageAuthViewDialog> mAuthDialog;
};
}

#endif // BoxJob_H
