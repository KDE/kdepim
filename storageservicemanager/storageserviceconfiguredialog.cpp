/*
  Copyright (c) 2014 Montel Laurent <montel.org>

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

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

StorageServiceConfigureDialog::StorageServiceConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Cancel | Ok  );
    mStorageSettings = new PimCommon::StorageServiceSettingsWidget;
    setMainWidget(mStorageSettings);
    readConfig();
}

StorageServiceConfigureDialog::~StorageServiceConfigureDialog()
{
    writeConfig();
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
