/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageserviceconfiguredialog.h"
#include "pimcommon/storageservice/settings/storageservicesettingswidget.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
#include "storageservicemanagerglobalconfig.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KUrlRequester>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

StorageServiceConfigureDialog::StorageServiceConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Cancel | Ok  );
    QWidget *w = new QWidget;
    setMainWidget(w);
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);
    mStorageSettings = new PimCommon::StorageServiceSettingsWidget;
    lay->addWidget(mStorageSettings);

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Default Download Folder:"));
    lay->addWidget(lab);
    mDownloadFolder = new KUrlRequester;
    mDownloadFolder->setMode(KFile::Directory|KFile::LocalOnly);
    lay->addWidget(mDownloadFolder);
    readConfig();
    loadSettings();
}

StorageServiceConfigureDialog::~StorageServiceConfigureDialog()
{
    writeConfig();
}

void StorageServiceConfigureDialog::loadSettings()
{
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mDownloadFolder, StorageServiceManagerGlobalConfig::self()->downloadDirectoryItem());
}

void StorageServiceConfigureDialog::writeSettings()
{
    PimCommon::ConfigureImmutableWidgetUtils::saveUrlRequester(mDownloadFolder, StorageServiceManagerGlobalConfig::self()->downloadDirectoryItem());
    StorageServiceManagerGlobalConfig::self()->writeConfig();
}

QMap<QString, PimCommon::StorageServiceAbstract *> StorageServiceConfigureDialog::listService() const
{
    return mStorageSettings->listService();
}

void StorageServiceConfigureDialog::setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst)
{
    mStorageSettings->setListService(lst);
}

void StorageServiceConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "StorageServiceConfigureDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServiceConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServiceConfigureDialog") );
    group.writeEntry( "Size", size() );
}
