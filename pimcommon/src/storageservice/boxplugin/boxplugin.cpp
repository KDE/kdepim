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

#include "boxplugin.h"
#include <kpluginfactory.h>
#include <KLocalizedString>
#include <QUrl>

using namespace PimCommon;
K_PLUGIN_FACTORY_WITH_JSON(PimCommonBoxPluginFactory, "pimcommon_boxplugin.json", registerPlugin<BoxPlugin>();)

BoxPlugin::BoxPlugin(QObject *parent, const QList<QVariant> &)
    : PimCommon::StorageServicePlugin(parent)
{

}

BoxPlugin::~BoxPlugin()
{

}

QString BoxPlugin::boxClientId() const
{
    //TODO
    return QString();
}

QString BoxPlugin::boxClientSecret() const
{
    //TODO
    return QString();
}

QString BoxPlugin::oauth2RedirectUrl() const
{
    //TODO
    return QString();
}

PimCommon::StorageServiceInterface *BoxPlugin::createStorageService(const QString &identifier)
{
    //TODO
    return Q_NULLPTR;
}

QString BoxPlugin::storageServiceName() const
{
    return QStringLiteral("box");
}

StorageServicePlugin::Capabilities BoxPlugin::capabilities() const
{
    StorageServicePlugin::Capabilities cap;
    cap |= AccountInfoCapability;
    //cap |= UploadFileCapability;
    cap |= DownloadFileCapability;
    cap |= CreateFolderCapability;
    cap |= DeleteFolderCapability;
    cap |= ListFolderCapability;
    cap |= DeleteFileCapability;
    cap |= ShareLinkCapability;
    cap |= RenameFolderCapability;
    cap |= RenameFileCapabilitity;
    cap |= MoveFileCapability;
    cap |= MoveFolderCapability;
    cap |= CopyFileCapability;
    cap |= CopyFolderCapability;
    return cap;
}

QString BoxPlugin::description() const
{
    return i18n("Box.com is a file hosting that offers cloud storage, file synchronization, and client software.");
}

QUrl BoxPlugin::serviceUrl() const
{
    return QUrl(QStringLiteral("https://app.box.com/"));
}

QString BoxPlugin::disallowedSymbols() const
{
    return QLatin1String("[/:?*\\|]");
}

QString BoxPlugin::disallowedSymbolsStr() const
{
    return QStringLiteral("\\ / : ? * < > |");
}

#include "boxplugin.moc"
