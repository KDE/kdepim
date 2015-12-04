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

#include "yousenditplugin.h"
#include <kpluginfactory.h>
#include <QUrl>
#include <KLocalizedString>

using namespace PimCommon;
K_PLUGIN_FACTORY_WITH_JSON(PimCommonYouSendItPluginFactory, "pimcommon_yousenditplugin.json", registerPlugin<YouSendItPlugin>();)

YouSendItPlugin::YouSendItPlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::StorageServicePlugin(parent)
{

}

YouSendItPlugin::~YouSendItPlugin()
{

}

QString YouSendItPlugin::youSendItApiKey() const
{
    return QString();
}

QString YouSendItPlugin::description() const
{
    return i18n("YouSendIt is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl YouSendItPlugin::serviceUrl() const
{
    return QUrl(QStringLiteral("https://www.yousendit.com/"));
}

QString YouSendItPlugin::storageServiceName() const
{
    return QStringLiteral("yousendit");
}

StorageServicePlugin::Capabilities YouSendItPlugin::capabilities() const
{
    StorageServicePlugin::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    //cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    //cap |= ShareLinkCapability;
    cap |= DeleteFileCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;

    //Can not be implemented.
    //cap |= CopyFileCapability;
    //cap |= CopyFolderCapability;

    return cap;
}

PimCommon::StorageServiceInterface *YouSendItPlugin::createStorageService(const QString &identifier)
{
    //TODO
    return Q_NULLPTR;
}

#include "yousenditplugin.moc"
