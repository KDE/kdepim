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

#ifndef GDriveJob_H
#define GDriveJob_H

#include <QObject>
#include "storageservice/job/storageserviceabstractjob.h"
#include <kgapi/account.h>

namespace KGAPI2
{
class Job;
}
namespace PimCommon
{
class GDriveJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit GDriveJob(QObject *parent = Q_NULLPTR);
    ~GDriveJob();

    void requestTokenAccess() Q_DECL_OVERRIDE;
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination) Q_DECL_OVERRIDE;
    void listFolder(const QString &folder = QString()) Q_DECL_OVERRIDE;
    void accountInfo() Q_DECL_OVERRIDE;
    void createFolder(const QString &foldername, const QString &destination) Q_DECL_OVERRIDE;
    void shareLink(const QString &root, const QString &path) Q_DECL_OVERRIDE;
    void initializeToken(KGAPI2::AccountPtr account);
    void createServiceFolder() Q_DECL_OVERRIDE;
    QNetworkReply *downloadFile(const QString &name, const QString &fileId, const QString &destination) Q_DECL_OVERRIDE;
    void deleteFile(const QString &filename) Q_DECL_OVERRIDE;
    void deleteFolder(const QString &foldername) Q_DECL_OVERRIDE;
    void renameFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void renameFile(const QString &oldName, const QString &newName) Q_DECL_OVERRIDE;
    void moveFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void moveFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFile(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    void copyFolder(const QString &source, const QString &destination) Q_DECL_OVERRIDE;
    virtual void refreshToken();

Q_SIGNALS:
    void authorizationDone(const QString &refreshToken, const QString &token, const QDateTime &expireTime, const QString &accountName);

private Q_SLOTS:
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
