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

#include "storageserviceconfigurewidget.h"
#include "pimcommon/storageservice/settings/storageservicesettingswidget.h"

#include <KLocalizedString>
#include <KUrlRequester>

#include <QHBoxLayout>
#include <QLabel>

using namespace PimCommon;

StorageServiceConfigureWidget::StorageServiceConfigureWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    mStorageSettings = new PimCommon::StorageServiceSettingsWidget;
    connect(mStorageSettings, &PimCommon::StorageServiceSettingsWidget::serviceRemoved, this, &StorageServiceConfigureWidget::serviceRemoved);
    connect(mStorageSettings, &PimCommon::StorageServiceSettingsWidget::changed, this, &StorageServiceConfigureWidget::changed);
    lay->addWidget(mStorageSettings);

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Default Download Folder:"));
    lay->addWidget(lab);
    mDownloadFolder = new KUrlRequester;
    connect(mDownloadFolder, &KUrlRequester::textChanged, this, &StorageServiceConfigureWidget::changed);
    mDownloadFolder->setMode(KFile::Directory|KFile::LocalOnly);
    lay->addWidget(mDownloadFolder);
}

StorageServiceConfigureWidget::~StorageServiceConfigureWidget()
{

}

void StorageServiceConfigureWidget::loadSettings()
{

}

void StorageServiceConfigureWidget::writeSettings()
{

}

StorageServiceSettingsWidget *StorageServiceConfigureWidget::storageServiceSettingsWidget() const
{
    return mStorageSettings;
}

KUrlRequester *StorageServiceConfigureWidget::downloadFolder() const
{
    return mDownloadFolder;
}
