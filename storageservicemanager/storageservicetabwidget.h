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

#ifndef STORAGESERVICETABWIDGET_H
#define STORAGESERVICETABWIDGET_H

#include <QTabWidget>
#include "pimcommon/storageservice/storageserviceabstract.h"
#include "pimcommon/storageservice/widgets/storageservicetreewidget.h"
class StorageServicePage;
class StorageServiceTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit StorageServiceTabWidget(QWidget *parent=0);
    ~StorageServiceTabWidget();

    PimCommon::StorageServiceAbstract::Capabilities capabilities() const;
    void setListStorageService(const QMap<QString, PimCommon::StorageServiceAbstract *> &list);
    void updateListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &list);
    bool hasUploadDownloadProgress() const;
    void serviceRemoved(const QString &serviceName);
    void setNetworkIsDown(bool state);
    bool listFolderWasLoaded() const;
    void logout();
    void shutdownAllServices();
    void refreshAll();
    PimCommon::StorageServiceTreeWidget::ItemType itemTypeSelected() const;

Q_SIGNALS:
    void updateStatusBarMessage(const QString &msg);
    void listFileWasInitialized();
    void selectionChanged();

public slots:
    void slotAuthenticate();
    void slotCreateFolder();
    void slotRefreshList();
    void slotAccountInfo();
    void slotUploadFile();
    void slotDeleteFile();
    void slotDownloadFile();
    void slotShowLog();

private slots:
    void slotUpdateIcon(const QIcon &icon, StorageServicePage *page);

private:
    void createPage(const QString &name, PimCommon::StorageServiceAbstract *service);
};

#endif // STORAGESERVICETABWIDGET_H
