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
#include "storageservice/widgets/storageserviceconfigurewidget.h"
#include "storageservice/settings/storageservicesettingswidget.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KUrlRequester>
#include <KGlobal>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

using namespace PimCommon;

StorageServiceConfigureDialog::StorageServiceConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Cancel | Ok  );
    mStorageServiceConfigureWidget = new PimCommon::StorageServiceConfigureWidget;
    connect(mStorageServiceConfigureWidget, SIGNAL(serviceRemoved(QString)), this, SIGNAL(serviceRemoved(QString)));
    setMainWidget(mStorageServiceConfigureWidget);
    readConfig();
}

StorageServiceConfigureDialog::~StorageServiceConfigureDialog()
{
    writeConfig();
}

void StorageServiceConfigureDialog::writeSettings()
{
    //Reimplement it
}

void StorageServiceConfigureDialog::loadSettings()
{
    //Reimplement it
}

QMap<QString, PimCommon::StorageServiceAbstract *> StorageServiceConfigureDialog::listService() const
{
    return mStorageServiceConfigureWidget->storageServiceSettingsWidget()->listService();
}

void StorageServiceConfigureDialog::setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst)
{
    mStorageServiceConfigureWidget->storageServiceSettingsWidget()->setListService(lst);
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

#include "moc_storageserviceconfiguredialog.cpp"
