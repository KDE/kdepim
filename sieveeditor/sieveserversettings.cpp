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

#include "sieveserversettings.h"

#include <kwallet.h>




class SieveServerSettingsPrivate
{
public:
    SieveServerSettingsPrivate()
        : sieveServerSettings( new SieveServerSettings )
    {
    }

    ~SieveServerSettingsPrivate()
    {
        delete sieveServerSettings;
    }

    SieveServerSettings *sieveServerSettings;
};

Q_GLOBAL_STATIC( SieveServerSettingsPrivate, sInstance )


SieveServerSettings::SieveServerSettings(QObject *parent)
    : QObject(parent),
      mWallet(0)
{
}

SieveServerSettings::~SieveServerSettings()
{
    delete mWallet;
}

SieveServerSettings *SieveServerSettings::self()
{
  return sInstance->sieveServerSettings; //will create it
}

KWallet::Wallet *SieveServerSettings::wallet()
{
    if (!mWallet) {
        mWallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
        if (mWallet) {
            connect(mWallet, &KWallet::Wallet::walletClosed, this, &SieveServerSettings::slotWalletClosed);
        }
    }
    return mWallet;
}

void SieveServerSettings::closeWallet()
{
    KWallet::Wallet::closeWallet(KWallet::Wallet::LocalWallet(), true);
}

void SieveServerSettings::slotWalletClosed()
{
    delete mWallet;
    mWallet = 0;
}
