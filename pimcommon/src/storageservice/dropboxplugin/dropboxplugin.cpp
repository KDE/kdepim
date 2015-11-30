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

#include "dropboxplugin.h"
#include <kpluginfactory.h>
#include <KLocalizedString>
#include <QUrl>
#include <QIcon>

using namespace PimCommon;
K_PLUGIN_FACTORY_WITH_JSON(PimCommonDropBoxPluginFactory, "pimcommon_dropboxplugin.json", registerPlugin<DropBoxPlugin>();)

DropBoxPlugin::DropBoxPlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::StorageServicePlugin(parent)
{

}

DropBoxPlugin::~DropBoxPlugin()
{

}

QString DropBoxPlugin::dropboxOauthConsumerKey() const
{
    //TODO
    return QString();
}

QString DropBoxPlugin::dropboxOauthSignature() const
{
    //TODO
    return QString();
}

QString DropBoxPlugin::dropboxRootPath() const
{
    //TODO
    return QString();
}

QString DropBoxPlugin::storageServiceName() const
{
    return QStringLiteral("dropbox");
}

StorageServicePlugin::Capabilities DropBoxPlugin::capabilities() const
{
    StorageServicePlugin::Capabilities cap;
    cap |= AccountInfoCapability;
    cap |= UploadFileCapability;
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ListFolderCapability;
    cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;

    return cap;
}

QString DropBoxPlugin::description() const
{
    return i18n("Dropbox is a file hosting service operated by Dropbox, Inc. that offers cloud storage, file synchronization, and client software.");
}

QUrl DropBoxPlugin::serviceUrl() const
{
    return QUrl(QStringLiteral("https://www.dropbox.com/"));
}

QString DropBoxPlugin::disallowedSymbols() const
{
    return QStringLiteral("[/:?*<>\"|]");
}

QString DropBoxPlugin::disallowedSymbolsStr() const
{
    return QStringLiteral("\\ / : ? * < > \" |");
}

qlonglong DropBoxPlugin::maximumUploadFileSize() const
{
    return 150000000;
}

QIcon DropBoxPlugin::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdepim-dropbox"));
}

PimCommon::StorageServiceInterface *DropBoxPlugin::createStorageService(const QString &identifier)
{
    //TODO
    return Q_NULLPTR;
}

#include "dropboxplugin.moc"
