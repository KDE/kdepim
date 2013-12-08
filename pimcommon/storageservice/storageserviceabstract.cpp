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

#include "storageserviceabstract.h"
#include <QDebug>

using namespace PimCommon;

StorageServiceAbstract::StorageServiceAbstract(QObject *parent)
    : QObject(parent)
{
}

StorageServiceAbstract::~StorageServiceAbstract()
{

}

void StorageServiceAbstract::slotAccountInfoDone(const PimCommon::AccountInfo &info)
{
    Q_EMIT accountInfoDone(storageServiceName(), info);
}

void StorageServiceAbstract::slotActionFailed(const QString &error)
{
    qDebug()<<" error found "<<error;
    Q_EMIT actionFailed(storageServiceName(), error);
}

void StorageServiceAbstract::slotShareLinkDone(const QString &url)
{
    Q_EMIT shareLinkDone(storageServiceName(), url);
}

void StorageServiceAbstract::slotUploadFileProgress(qint64 done, qint64 total)
{
    Q_EMIT uploadFileProgress(storageServiceName(), done, total);
}

void StorageServiceAbstract::slotCreateFolderDone()
{
    Q_EMIT createFolderDone(storageServiceName());
}

void StorageServiceAbstract::slotUploadFileDone()
{
    Q_EMIT uploadFileDone(storageServiceName());
}

void StorageServiceAbstract::slotListFolderDone()
{
    Q_EMIT listFolderDone(storageServiceName());
}


#include "moc_storageserviceabstract.cpp"
