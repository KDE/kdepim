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

#ifndef STORAGESERVICEABSTRACT_H
#define STORAGESERVICEABSTRACT_H

#include <QObject>
#include <QUrl>
#include <KIcon>

namespace PimCommon {

struct AccountInfo {
    AccountInfo()
        : accountSize(-1),
          quota(-1),
          shared(-1)
    {

    }
    qint64 accountSize;
    qint64 quota;
    qint64 shared;
    QString displayName;
};

class StorageServiceAbstract : public QObject
{
    Q_OBJECT
public:
    explicit StorageServiceAbstract(QObject *parent=0);
    ~StorageServiceAbstract();

    enum ActionType {
        NoneAction = 0,
        //Account
        RequestToken,
        AccessToken,
        AccountInfo,
        //File
        UploadFile,
        DownLoadFile,
        ShareLink,
        DeleteFile,
        //Folder
        CreateFolder,
        ListFolder,
        CreateServiceFolder,
        DeleteFolder
    };

    bool isInProgress() const;

    virtual void downloadFile(const QString &filename);
    virtual void uploadFile(const QString &filename);
    virtual void accountInfo();
    virtual void createFolder(const QString &folder);
    virtual void listFolder();
    virtual void authentication();
    virtual void shareLink(const QString &root, const QString &path);
    virtual void createServiceFolder();
    virtual void deleteFile(const QString &filename);
    virtual void deleteFolder(const QString &foldername);


    virtual QString storageServiceName() const = 0;
    virtual KIcon icon() const = 0;
    virtual void removeConfig() = 0;

Q_SIGNALS:
    void actionFailed(const QString &serviceName, const QString &error);
    void accountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &);
    void uploadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void shareLinkDone(const QString &serviceName, const QString &link);
    void authenticationDone(const QString &serviceName);
    void authenticationFailed(const QString &serviceName, const QString &error = QString());
    void createFolderDone(const QString &serviceName, const QString &folderName);
    void uploadFileDone(const QString &serviceName, const QString &fileName);
    void listFolderDone(const QString &serviceName, const QStringList &listFolder);
    void downLoadFileDone(const QString &serviceName, const QString &fileName);
    void deleteFolderDone(const QString &serviceName, const QString &folder);
    void deleteFileDone(const QString &serviceName, const QString &filename);

protected slots:
    void slotActionFailed(const QString &error);
    void slotAccountInfoDone(const PimCommon::AccountInfo &info);
    void slotShareLinkDone(const QString &url);
    void slotUploadFileProgress(qint64 done, qint64 total);
    void slotCreateFolderDone(const QString &folderName);
    void slotUploadFileDone(const QString &filename);
    void slotListFolderDone(const QStringList &listFolder);
    void slotDownLoadFileDone(const QString &fileName);    
    void slotDeleteFolderDone(const QString &folder);
    void slotDeleteFileDone(const QString &filename);
protected:
    virtual void storageServicedownloadFile(const QString &filename) = 0;
    virtual void storageServiceuploadFile(const QString &filename) = 0;
    virtual void storageServiceaccountInfo() = 0;
    virtual void storageServicecreateFolder(const QString &folder) = 0;
    virtual void storageServicelistFolder() = 0;
    virtual void storageServiceauthentication() = 0;
    virtual void storageServiceShareLink(const QString &root, const QString &path) = 0;
    virtual void storageServicecreateServiceFolder() = 0;
    virtual void storageServicedeleteFile(const QString &filename) = 0;
    virtual void storageServicedeleteFolder(const QString &foldername) = 0;
    void emitAuthentificationDone();
    void emitAuthentificationFailder(const QString &errorMessage);
    ActionType mNextAction;

private:
    void executeNextAction();
    bool mInProgress;
};
}

#endif // STORAGESERVICEABSTRACT_H
