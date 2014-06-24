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
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
#include "pimcommon/storageservice/widgets/storageserviceconfigurewidget.h"
#include "storageservicemanagerglobalconfig.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QCheckBox>
#include <QLayout>

StorageServiceConfigureDialog::StorageServiceConfigureDialog(QWidget *parent)
    : PimCommon::StorageServiceConfigureDialog(parent)
{
    mCloseWallet = new QCheckBox(i18n("Close wallet when close application"));
    mainWidget()->layout()->addWidget(mCloseWallet);
    loadSettings();
}

StorageServiceConfigureDialog::~StorageServiceConfigureDialog()
{
}

void StorageServiceConfigureDialog::loadSettings()
{
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mStorageServiceConfigureWidget->downloadFolder(), StorageServiceManagerGlobalConfig::self()->downloadDirectoryItem());
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mCloseWallet, StorageServiceManagerGlobalConfig::self()->closeWalletItem());
}

void StorageServiceConfigureDialog::writeSettings()
{
    PimCommon::ConfigureImmutableWidgetUtils::saveUrlRequester(mStorageServiceConfigureWidget->downloadFolder(), StorageServiceManagerGlobalConfig::self()->downloadDirectoryItem());
    PimCommon::ConfigureImmutableWidgetUtils::saveCheckBox(mCloseWallet, StorageServiceManagerGlobalConfig::self()->closeWalletItem());

    StorageServiceManagerGlobalConfig::self()->save();
}

