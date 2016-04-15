/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef STORAGESERVICEMANAGERMAINWINDOW_H
#define STORAGESERVICEMANAGERMAINWINDOW_H

#include <KXmlGuiWindow>

class QNetworkConfigurationManager;

namespace PimCommon
{
class StorageServiceManager;
}

class QAction;
class QLabel;
class StorageServiceManagerMainWidget;
class StorageServiceManagerMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit StorageServiceManagerMainWindow();
    ~StorageServiceManagerMainWindow();

public Q_SLOTS:
    void slotSetStatusBarMessage(const QString &message);

protected:
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotConfigure();
    void slotUpdateActions();
    void slotServiceRemoved(const QString &serviceName);
    void slotLogout();
    void slotShutdownAllServices();
    void slotRefreshAll();
    void slotShowNotificationOptions();
    void slotServicesChanged();
    void slotSystemNetworkOnlineStateChanged(bool state);
private:
    void setupActions();
    void readConfig();
    void initStatusBar();
    StorageServiceManagerMainWidget *mStorageServiceMainWidget;
    PimCommon::StorageServiceManager *mStorageManager;
    QAction *mDownloadFile;
    QAction *mCreateFolder;
    QAction *mAccountInfo;
    QAction *mUploadFile;
    QAction *mDelete;
    QAction *mAuthenticate;
    QAction *mRefreshList;
    QAction *mShowLog;
    QAction *mLogout;
    QAction *mShutdownAllServices;
    QAction *mRefreshAll;
    QAction *mRenameItem;
    QLabel *mStatusBarInfo;
    QNetworkConfigurationManager *mNetworkConfigurationManager;
};

#endif // STORAGESERVICEMANAGERMAINWINDOW_H
