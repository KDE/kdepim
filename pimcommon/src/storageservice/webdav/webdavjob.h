/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef WEBDAVJOB_H
#define WEBDAVJOB_H

#include <QObject>
#include "storageservice/job/storageserviceabstractjob.h"
class QNetworkReply;
namespace PimCommon
{
class WebDavJob : public PimCommon::StorageServiceAbstractJob
{
    Q_OBJECT
public:
    explicit WebDavJob(QObject *parent = Q_NULLPTR);
    ~WebDavJob();

    void requestTokenAccess() Q_DECL_OVERRIDE;
    QNetworkReply *uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination) Q_DECL_OVERRIDE;
    void listFolder(const QString &folder = QString()) Q_DECL_OVERRIDE;
    void accountInfo() Q_DECL_OVERRIDE;
    void createFolder(const QString &foldername, const QString &destination) Q_DECL_OVERRIDE;
    void shareLink(const QString &root, const QString &path) Q_DECL_OVERRIDE;
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
    void initializeToken(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password);

Q_SIGNALS:
    void authorizationDone(const QString &publicLocation, const QString &serviceLocation, const QString &username, const QString &password);

private Q_SLOTS:
    void slotSendDataFinished(QNetworkReply *reply);
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *);
    void slotListInfo(const QString &data);

private:
    //QWebDav API
    typedef QMap < QString, QMap < QString, QVariant > > PropValues;
    typedef QMap < QString, QStringList > PropNames;

    QNetworkReply *list(const QString &dir);
    QNetworkReply *search(const QString &path, const QString &q);
    QNetworkReply *put(const QString &path, QIODevice *data);
    QNetworkReply *put(const QString &path, QByteArray &data);
    QNetworkReply *propfind(const QUrl &path, const WebDavJob::PropNames &props, int depth);
    QNetworkReply *propfind(const QUrl &path, const QByteArray &query, int depth);
    QNetworkReply *proppatch(const QUrl &path, const WebDavJob::PropValues &props);
    QNetworkReply *proppatch(const QUrl &path, const QByteArray &query);
    QNetworkReply *davRequest(const QString &reqVerb, QNetworkRequest &req, const QByteArray &data = QByteArray());
    QNetworkReply *davRequest(const QString &reqVerb, QNetworkRequest &req, QIODevice *data);
    QNetworkReply *mkdir(const QUrl &dir);
    QNetworkReply *copy(const QString &oldname, const QString &newname, bool overwrite);
    QNetworkReply *rename(const QString &oldname, const QString &newname, bool overwrite);
    QNetworkReply *move(const QString &oldname, const QString &newname, bool overwrite);
    QNetworkReply *rmdir(const QUrl &dir);
    QNetworkReply *remove(const QUrl &path);
    QNetworkReply *accountInfo(const QString &dir);
    QNetworkReply *exists(const QUrl &dir);
    void setupHeaders(QNetworkRequest &req, quint64 size);

    void parseDownloadFile(const QString &data);
    void parseUploadFile(const QString &data);
    void parseCreateFolder(const QString &data);
    void parseAccountInfo(const QString &data);
    void parseListFolder(const QString &data);
    void parseAccessToken(const QString &data);
    void parseDeleteFile(const QString &data);
    void parseDeleteFolder(const QString &data);
    void parseRenameFolder(const QString &data);
    void parseRenameFile(const QString &data);
    void parseMoveFolder(const QString &data);
    void parseMoveFile(const QString &data);
    void parseCopyFolder(const QString &data);
    void parseCopyFile(const QString &data);
    void parseShareLink(const QString &data);
    void parseCreateServiceFolder(const QString &data);

    void createFolderJob(const QString &foldername, const QString &destination);

    QString mPublicLocation;
    QString mServiceLocation;
    QString mUserName;
    QString mPassword;
    QString mShareApi;

    int mNbAuthCheck;
};
}

#endif // WEBDAVJOB_H
