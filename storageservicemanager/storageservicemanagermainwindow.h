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

namespace PimCommon {
class StorageServiceManager;
}
class KAction;
class StorageServiceTabWidget;
class StorageServiceManagerMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit StorageServiceManagerMainWindow();
    ~StorageServiceManagerMainWindow();

private slots:
    void slotQuitApp();    
    void slotAddStorageService();    
    void slotConfigure();        
    void slotUpdateActions();

private:
    void setupActions();
    void readConfig();
    StorageServiceTabWidget *mStorageServiceTabWidget;
    PimCommon::StorageServiceManager *mStorageManager;
    KAction *mDownloadFile;
    KAction *mCreateFolder;
    KAction *mAccountInfo;
    KAction *mUploadFile;
    KAction *mDeleteFile;
};

#endif // STORAGESERVICEMANAGERMAINWINDOW_H
