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

#include "storageservicesettings.h"

#include <kwallet.h>

#include <KGlobal>

namespace PimCommon {
class StorageServiceSettingsPrivate
{
public:
    StorageServiceSettingsPrivate()
        : storageServiceSettings( new StorageServiceSettings )
    {
    }

    ~StorageServiceSettingsPrivate()
    {
        delete storageServiceSettings;
    }

    StorageServiceSettings *storageServiceSettings;
};

K_GLOBAL_STATIC( StorageServiceSettingsPrivate, sInstance )


StorageServiceSettings::StorageServiceSettings(QObject *parent)
    : QObject(parent),
      mWallet(0)
{
}

StorageServiceSettings::~StorageServiceSettings()
{
    delete mWallet;
}

StorageServiceSettings *StorageServiceSettings::self()
{
  return sInstance->storageServiceSettings; //will create it
}

KWallet::Wallet *StorageServiceSettings::wallet()
{
    if (!mWallet) {
        mWallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
        if (mWallet) {
            connect(mWallet, SIGNAL(walletClosed()), this, SLOT(slotWalletClosed()));
        }
    }
    return mWallet;
}

bool StorageServiceSettings::createDefaultFolder()
{
    KWallet::Wallet *wallet = StorageServiceSettings::self()->wallet();
    const QString folderName = QLatin1String("storageservice");
    if ( wallet ) {
        if (!wallet->setFolder( folderName ) ) {
            wallet->createFolder(folderName);
            wallet->setFolder(folderName);
        }
        return true;
    } else {
        return false;
    }
}

void StorageServiceSettings::slotWalletClosed()
{
    delete mWallet;
    mWallet = 0;
}


}
#include "moc_storageservicesettings.cpp"
