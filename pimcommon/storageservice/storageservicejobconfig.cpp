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

#include "storageservicejobconfig.h"
#include "storageserviceinterface.h"


#include <KGlobal>

#include <QDebug>

namespace PimCommon {

class StorageServiceJobConfigPrivate
{
public:
    StorageServiceJobConfigPrivate()
        : storageServiceJobConfig( new PimCommon::StorageServiceJobConfig )
    {
    }

    ~StorageServiceJobConfigPrivate()
    {
        qDebug()<<" ~StorageServiceJobConfigPrivate ";
        delete storageServiceJobConfig;
    }
    StorageServiceJobConfig *storageServiceJobConfig;
};

K_GLOBAL_STATIC( StorageServiceJobConfigPrivate, sInstance )


StorageServiceJobConfig::StorageServiceJobConfig(QObject *parent)
    : QObject(parent),
      mSettingsJob(0)
{
}

StorageServiceJobConfig::~StorageServiceJobConfig()
{

}

StorageServiceJobConfig *StorageServiceJobConfig::self()
{
  return sInstance->storageServiceJobConfig; //will create it
}

void StorageServiceJobConfig::registerConfigIf(ISettingsJob *settingsJob)
{
    mSettingsJob = settingsJob;
}

QString StorageServiceJobConfig::youSendItApiKey() const
{
    if (!mSettingsJob) {
        qDebug()<<" settings job not registered";
        return QString();
    }
    return mSettingsJob->youSendItApiKey();
}

}
