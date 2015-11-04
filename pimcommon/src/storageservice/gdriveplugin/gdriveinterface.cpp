/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "gdriveinterface.h"

using namespace PimCommon;

GDriveInterface::GDriveInterface(GDrivePlugin *plugin, QObject *parent)
    : PimCommon::StorageServiceInterface(parent),
      mPlugin(plugin)
{

}

GDriveInterface::~GDriveInterface()
{

}

void GDriveInterface::shutdownService()
{

}

bool GDriveInterface::isConfigurated() const
{
    //TODO
    return true;
}

void GDriveInterface::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{

}

void GDriveInterface::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{

}

void GDriveInterface::accountInfo()
{

}

void GDriveInterface::createFolder(const QString &foldername, const QString &destination)
{

}

void GDriveInterface::listFolder(const QString &folder)
{

}

void GDriveInterface::authentication()
{

}

void GDriveInterface::shareLink(const QString &root, const QString &path)
{

}

void GDriveInterface::createServiceFolder()
{

}

void GDriveInterface::deleteFile(const QString &filename)
{

}

void GDriveInterface::deleteFolder(const QString &foldername)
{

}

void GDriveInterface::renameFolder(const QString &source, const QString &destination)
{

}

void GDriveInterface::renameFile(const QString &source, const QString &destination)
{

}

void GDriveInterface::moveFile(const QString &source, const QString &destination)
{

}

void GDriveInterface::moveFolder(const QString &source, const QString &destination)
{

}

void GDriveInterface::copyFile(const QString &source, const QString &destination)
{

}

void GDriveInterface::copyFolder(const QString &source, const QString &destination)
{

}

void GDriveInterface::removeConfig()
{

}
