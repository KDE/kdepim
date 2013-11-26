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

#include "dropboxstorageservice.h"

#include <KLocale>

using namespace PimCommon;

DropBoxStorageService::DropBoxStorageService(QObject *parent)
    : PimCommon::StockageServiceAbstract(parent)
{
}

DropBoxStorageService::~DropBoxStorageService()
{

}

QString DropBoxStorageService::name() const
{
    return i18n("DropBox");
}

qint64 DropBoxStorageService::maximumSize() const
{
    return -1;
}

qint64 DropBoxStorageService::currentSize() const
{
    return -1;
}

QUrl DropBoxStorageService::sharedUrl() const
{
    return QUrl();
}

void DropBoxStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString DropBoxStorageService::description() const
{
    return QString();
}
