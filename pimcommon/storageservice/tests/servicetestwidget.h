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


#ifndef SERVICETESTWIDGET_H
#define SERVICETESTWIDGET_H

#include <QWidget>
class QTextEdit;

namespace PimCommon {
class StorageServiceAbstract;
class AccountInfo;
}
class ServiceTestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ServiceTestWidget(PimCommon::StorageServiceAbstract *, QWidget *parent);
    ~ServiceTestWidget();

private Q_SLOTS:
    void slotListFolder();

    void slotCreateFolder();
    void slotAccountInfo();

    void slotActionFailed(const QString &serviceName, const QString &error);
    void slotUploadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotShareLinkDone(const QString &serviceName, const QString &shareLink);
    void slotAuthentificationDone(const QString &serviceName);
    void slotCreateFolderDone(const QString &serviceName);
    void slotUploadFileDone(const QString &serviceName);
    void slotListFolderDone(const QString &serviceName);
    void slotAccountInfoDone(const QString &serviceName, const PimCommon::AccountInfo &info);
    void slotUploadFile();
    void slotAuthentification();
private:
    void connectStorageService();
    PimCommon::StorageServiceAbstract *mStorageService;
    QTextEdit *mEdit;
};

#endif // SERVICETESTWIDGET_H
