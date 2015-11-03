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

#include "hubicinterface.h"

using namespace PimCommon;

HubicInterface::HubicInterface(HubicPlugin *plugin, QObject *parent)
    : PimCommon::StorageServiceInterface(parent),
      mPlugin(plugin)
{

}

HubicInterface::~HubicInterface()
{

}

void HubicInterface::shutdownService()
{

}

bool HubicInterface::isConfigurated() const
{
    //TODO
    return true;
}

void HubicInterface::downloadFile(const QString &name, const QString &fileId, const QString &destination)
{

}

void HubicInterface::uploadFile(const QString &filename, const QString &uploadAsName, const QString &destination)
{

}

void HubicInterface::accountInfo()
{

}

void HubicInterface::createFolder(const QString &foldername, const QString &destination)
{

}

void HubicInterface::listFolder(const QString &folder)
{

}

void HubicInterface::authentication()
{

}

void HubicInterface::shareLink(const QString &root, const QString &path)
{

}

void HubicInterface::createServiceFolder()
{

}

void HubicInterface::deleteFile(const QString &filename)
{

}

void HubicInterface::deleteFolder(const QString &foldername)
{

}

void HubicInterface::renameFolder(const QString &source, const QString &destination)
{

}

void HubicInterface::renameFile(const QString &source, const QString &destination)
{

}

void HubicInterface::moveFile(const QString &source, const QString &destination)
{

}

void HubicInterface::moveFolder(const QString &source, const QString &destination)
{

}

void HubicInterface::copyFile(const QString &source, const QString &destination)
{

}

void HubicInterface::copyFolder(const QString &source, const QString &destination)
{

}

void HubicInterface::removeConfig()
{

}
