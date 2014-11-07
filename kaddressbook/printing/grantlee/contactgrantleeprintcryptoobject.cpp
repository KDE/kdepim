/*
  Copyright (c)2014 Montel Laurent <montel@kde.org>

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

#include "contactgrantleeprintcryptoobject.h"
#include "libkleo/kleo/enum.h"

using namespace KABPrinting;
static QString loadCustom(const KContacts::Addressee &contact, const QString &key)
{
    return contact.custom(QLatin1String("KADDRESSBOOK"), key);
}

ContactGrantleePrintCryptoObject::ContactGrantleePrintCryptoObject(const KContacts::Addressee &address, QObject *parent)
    : QObject(parent),
      mAddress(address)
{
}

ContactGrantleePrintCryptoObject::~ContactGrantleePrintCryptoObject()
{

}

QString ContactGrantleePrintCryptoObject::signaturePreference() const
{
    return Kleo::signingPreferenceToLabel(Kleo::stringToSigningPreference(loadCustom(mAddress, QLatin1String("CRYPTOSIGNPREF"))));
}

QString ContactGrantleePrintCryptoObject::cryptoPreference() const
{
    return Kleo::encryptionPreferenceToLabel(Kleo::stringToEncryptionPreference(loadCustom(mAddress, QLatin1String("CRYPTOENCRYPTPREF"))));
}
