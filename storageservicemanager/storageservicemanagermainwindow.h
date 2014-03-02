/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include <Solid/Networking>

namespace PimCommon {
class StorageServiceManager;
}

class KAction;
class QLabel;
class StorageServiceTabWidget;
class StorageServiceManagerMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit StorageServiceManagerMainWindow();
    ~StorageServiceManagerMainWindow();

public slots:
    void slotSetStatusBarMessage(const QString &message);

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void slotConfigure();
    void slotUpdateActions();
    void slotSystemNetworkStatusChanged(Solid::Networking::Status status);
    void slotServiceRemoved(const QString &serviceName);
    void slotLogout();
    void slotShutdownAllServices();
    void slotRefreshAll();
    void slotShowNotificationOptions();
    void slotServicesChanged();
private:
    void setupActions();
    void readConfig();
    void initStatusBar();
    StorageServiceTabWidget *mStorageServiceTabWidget;
    PimCommon::StorageServiceManager *mStorageManager;
    KAction *mDownloadFile;
    KAction *mCreateFolder;
    KAction *mAccountInfo;
    KAction *mUploadFile;
    KAction *mDelete;
    KAction *mAuthenticate;
    KAction *mRefreshList;
    KAction *mShowLog;
    KAction *mLogout;
    KAction *mShutdownAllServices;
    KAction *mRefreshAll;
    KAction *mRenameItem;
    QLabel *mStatusBarInfo;
    bool mNetworkIsDown;
};

#endif // STORAGESERVICEMANAGERMAINWINDOW_H
