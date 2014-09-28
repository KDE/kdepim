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

#include "contactgrantleeprintgeoobject.h"

using namespace KABPrinting;

ContactGrantleePrintGeoObject::ContactGrantleePrintGeoObject(const KABC::Geo &geo, QObject *parent)
    : QObject(parent),
      mGeo(geo)
{

}

ContactGrantleePrintGeoObject::~ContactGrantleePrintGeoObject()
{

}

float ContactGrantleePrintGeoObject::latitude() const
{
    if (mGeo.isValid()) {
        return mGeo.latitude();
    }
    return -1;
}

float ContactGrantleePrintGeoObject::longitude() const
{
    if (mGeo.isValid()) {
        return mGeo.longitude();
    }
    return -1;
}

QString ContactGrantleePrintGeoObject::toString() const
{
    return mGeo.toString();
}

bool ContactGrantleePrintGeoObject::isValid() const
{
    return mGeo.isValid();
}
