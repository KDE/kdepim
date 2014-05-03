/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "contactgrantleeprintaddressobject.h"
#include <QDebug>

using namespace KABPrinting;
ContactGrantleePrintAddressObject::ContactGrantleePrintAddressObject(const KABC::Address &address, QObject *parent)
    : QObject(parent),
      mAddress(address)
{
}

ContactGrantleePrintAddressObject::~ContactGrantleePrintAddressObject()
{
}

QString ContactGrantleePrintAddressObject::type() const
{
    return KABC::Address::typeLabel( mAddress.type() );
}

QString ContactGrantleePrintAddressObject::street() const
{
    return mAddress.street();
}

QString ContactGrantleePrintAddressObject::postOfficeBox() const
{
    return mAddress.postOfficeBox();
}

QString ContactGrantleePrintAddressObject::locality() const
{
    return mAddress.locality();
}

QString ContactGrantleePrintAddressObject::region() const
{
    return mAddress.region();
}

QString ContactGrantleePrintAddressObject::postalCode() const
{
    return mAddress.postalCode();
}

QString ContactGrantleePrintAddressObject::country() const
{
    return mAddress.country();
}

QString ContactGrantleePrintAddressObject::label() const
{
    return mAddress.label();
}

QString ContactGrantleePrintAddressObject::formattedAddress() const
{
    return mAddress.formattedAddress().replace(QLatin1String("\n"), QLatin1String("<br>"));
}
