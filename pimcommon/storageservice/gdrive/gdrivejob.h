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

#ifndef GDriveJob_H
#define GDriveJob_H

#include <QObject>
#include "storageservice/job/storageserviceabstractjob.h"
#include <libkgapi2/account.h>

namespace KGAPI2 {
class Job;
}
namespace PimCommon {
class GDriveJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit GDriveJob(QObject *parent=0);
    ~GDriveJob();

    void requestTokenAccess();
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination);
    void listFolder(const QString &folder = QString());
    void accountInfo();
    void createFolder(const QString &foldername, const QString &destination);
    void shareLink(const QString &root, const QString &path);
    void initializeToken(KGAPI2::AccountPtr account);
    void createServiceFolder();
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination);
    void deleteFile(const QString &filename);
    void deleteFolder(const QString &foldername);
    void renameFolder(const QString &source, const QString &destination);
    void renameFile(const QString &oldName, const QString &newName);
    void moveFolder(const QString &source, const QString &destination);
    void moveFile(const QString &source, const QString &destination);
    void copyFile(const QString &source, const QString &destination);
    void copyFolder(const QString &source, const QString &destination);
    virtual void refreshToken();

Q_SIGNALS:
    void authorizationDone(const QString &refreshToken, const QString &token, const QDateTime &expireTime, const QString &accountName);

private slots:
    void slotAuthJobFinished(KGAPI2::Job *job);
    void slotAboutFetchJobFinished(KGAPI2::Job *job);
    void slotDeleteFileFinished(KGAPI2::Job *job);
    void slotFileFetchFinished(KGAPI2::Job *job);
    void slotChildReferenceFetchJobFinished(KGAPI2::Job *job);
    void slotDeleteFolderFinished(KGAPI2::Job *job);
    void slotCreateJobFinished(KGAPI2::Job *job);
    void slotUploadJobFinished(KGAPI2::Job *job);
    void slotUploadDownLoadProgress(KGAPI2::Job *job, int progress, int total);
    void slotCopyJobFinished(KGAPI2::Job *);
    void slotCopyFolderJobFinished(KGAPI2::Job *job);
private:
    bool handleError(KGAPI2::Job *job);

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
    QString lastPathComponent(const QUrl &url) const;
    KGAPI2::AccountPtr mAccount;
};
}
#endif // GDriveJob_H
