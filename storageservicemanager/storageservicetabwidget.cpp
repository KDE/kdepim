/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

StorageServiceTabWidget::StorageServiceTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
}

StorageServiceTabWidget::~StorageServiceTabWidget()
{

}

void StorageServiceTabWidget::loadStorageService()
{
    //TODO
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
            page->uploadFile();
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
