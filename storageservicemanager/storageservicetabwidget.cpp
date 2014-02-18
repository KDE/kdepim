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

#include "storageservicetabwidget.h"
#include "storageservicepage.h"
#include "storageservice/storageserviceabstract.h"

#include <QMap>

StorageServiceTabWidget::StorageServiceTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
}

StorageServiceTabWidget::~StorageServiceTabWidget()
{

}

void StorageServiceTabWidget::updateListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &list)
{
    QMapIterator<QString, PimCommon::StorageServiceAbstract*> i(list);
    while (i.hasNext()) {
        i.next();
        bool foundPage = false;
        StorageServicePage *page = 0;
        for (int nbPage=0; nbPage < count(); ++nbPage) {
            page = static_cast<StorageServicePage*>(widget(nbPage));
            if (i.value()->storageServiceName() == page->serviceName()) {
                foundPage = true;
                break;
            }
        }
        if (!foundPage) {
            createPage(i.key(), i.value());
        } else {
            if (page) {
                page->refreshList();
            }
        }
    }
}

void StorageServiceTabWidget::setListStorageService(const QMap<QString, PimCommon::StorageServiceAbstract *> &list)
{
    QMapIterator<QString, PimCommon::StorageServiceAbstract*> i(list);
    while (i.hasNext()) {
        i.next();
        createPage(i.key(), i.value());
    }
}

void StorageServiceTabWidget::createPage(const QString &name, PimCommon::StorageServiceAbstract *service)
{
    StorageServicePage *page = new StorageServicePage(name, service);
    connect(page, SIGNAL(updateIcon(QIcon,StorageServicePage*)), this, SLOT(slotUpdateIcon(QIcon,StorageServicePage*)));
    connect(page, SIGNAL(updateStatusBarMessage(QString)), this, SIGNAL(updateStatusBarMessage(QString)));
    connect(page, SIGNAL(listFileWasInitialized()), this, SIGNAL(listFileWasInitialized()));
    addTab(page, name);
}

void StorageServiceTabWidget::slotUpdateIcon(const QIcon &icon, StorageServicePage *page)
{
    if (page) {
        const int index = indexOf(page);
        if (index != -1) {
            setTabIcon(index, icon);
        }
    }
}

void StorageServiceTabWidget::slotAuthenticate()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->authenticate();
    }
}

void StorageServiceTabWidget::slotCreateFolder()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->createFolder();
    }
}

void StorageServiceTabWidget::slotRefreshList()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->refreshList();
    }
}

void StorageServiceTabWidget::slotAccountInfo()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->accountInfo();
    }
}

void StorageServiceTabWidget::slotUploadFile()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->slotUploadFile();
    }
}

void StorageServiceTabWidget::slotDeleteFile()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->deleteFile();
    }
}

void StorageServiceTabWidget::slotDownloadFile()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->downloadFile();
    }
}

PimCommon::StorageServiceAbstract::Capabilities StorageServiceTabWidget::capabilities() const
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            return page->capabilities();
    }
    return PimCommon::StorageServiceAbstract::NoCapability;
}

bool StorageServiceTabWidget::listFolderWasLoaded() const
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            return page->listFolderWasLoaded();
    }
    return false;
}

bool StorageServiceTabWidget::hasUploadDownloadProgress() const
{
    for (int i=0; i <count() ; ++i) {
        StorageServicePage *page = static_cast<StorageServicePage *>(widget(i));
        if (page) {
            if (page->hasUploadDownloadProgress())
                return true;
        }
    }
    return false;
}

void StorageServiceTabWidget::serviceRemoved(const QString &serviceName)
{
    for (int nbPage=0; nbPage < count(); ++nbPage) {
        StorageServicePage *page = static_cast<StorageServicePage*>(widget(nbPage));
        if (page->serviceName() == serviceName) {
            //removeTab(nbPage);
            delete widget(nbPage);
            break;
        }
    }
}


void StorageServiceTabWidget::setNetworkIsDown(bool state)
{
    for (int i=0; i <count() ; ++i) {
        StorageServicePage *page = static_cast<StorageServicePage *>(widget(i));
        if (page) {
            page->setNetworkIsDown(state);
        }
    }
}

void StorageServiceTabWidget::slotShowLog()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->showLog();
    }
}

void StorageServiceTabWidget::logout()
{
    if (currentWidget()) {
        StorageServicePage *page = static_cast<StorageServicePage *>(currentWidget());
        if (page)
            page->logout();
    }
}

void StorageServiceTabWidget::shutdownAllServices()
{
    for (int i=0; i <count() ; ++i) {
        StorageServicePage *page = static_cast<StorageServicePage *>(widget(i));
        if (page) {
            page->logout();
        }
    }
}
