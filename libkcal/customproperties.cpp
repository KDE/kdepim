/*
    This file is part of libkcal.

    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "customproperties.h"

#include <kdebug.h>
//Added by qt3to4:
#include <QByteArray>

using namespace KCal;

CustomProperties::CustomProperties()
{
}

CustomProperties::CustomProperties(const CustomProperties &cp)
  : mProperties(cp.mProperties)
{
}

CustomProperties::~CustomProperties()
{
}

bool CustomProperties::operator==( const CustomProperties &other ) const
{
  if ( mProperties.count() != other.mProperties.count() ) return false;
  QMap<QByteArray, QString>::ConstIterator it;
  for( it = mProperties.begin(); it != mProperties.end(); ++it ) {
    QMap<QByteArray, QString>::ConstIterator itOther =
      other.mProperties.find( it.key() );

    if ( itOther == other.mProperties.end() ) {
      return false;
    }
    if ( itOther.value() != it.value() ) return false;
  }

  return true;
}

void CustomProperties::setCustomProperty(const QByteArray &app, const QByteArray &key,
                                         const QString &value)
{
  if (value.isNull() || key.isEmpty() || app.isEmpty())
    return;
  QByteArray property = "X-KDE-" + app + "-" + key;
  if (!checkName(property))
    return;
  mProperties[property] = value;
}

void CustomProperties::removeCustomProperty(const QByteArray &app, const QByteArray &key)
{
  removeNonKDECustomProperty(QByteArray("X-KDE-" + app + "-" + key));
}

QString CustomProperties::customProperty(const QByteArray &app, const QByteArray &key) const
{
  return nonKDECustomProperty(QByteArray("X-KDE-" + app + "-" + key));
}

void CustomProperties::setNonKDECustomProperty(const QByteArray &name, const QString &value)
{
  if (value.isNull() || !checkName(name))
    return;
  mProperties[name] = value;
}

void CustomProperties::removeNonKDECustomProperty(const QByteArray &name)
{
  QMap<QByteArray, QString>::Iterator it = mProperties.find(name);
  if (it != mProperties.end())
    mProperties.erase(it);
}

QString CustomProperties::nonKDECustomProperty(const QByteArray &name) const
{
  QMap<QByteArray, QString>::ConstIterator it = mProperties.find(name);
  if (it == mProperties.end())
    return QString();
  return it.value();
}

void CustomProperties::setCustomProperties(const QMap<QByteArray, QString> &properties)
{
  for (QMap<QByteArray, QString>::ConstIterator it = properties.begin();  it != properties.end();  ++it) {
    // Validate the property name and convert any null string to empty string
    if (checkName(it.key())) {
      mProperties[it.key()] = it.value().isNull() ? QString("") : it.value();
    }
  }
}

QMap<QByteArray, QString> CustomProperties::customProperties() const
{
  return mProperties;
}

bool CustomProperties::checkName(const QByteArray &name)
{
  // Check that the property name starts with 'X-' and contains
  // only the permitted characters
  const char* n = name;
  int len = name.length();
  if (len < 2 ||  n[0] != 'X' || n[1] != '-')
    return false;
  for (int i = 2;  i < len;  ++i) {
    char ch = n[i];
    if (ch >= 'A' && ch <= 'Z'
    ||  ch >= 'a' && ch <= 'z'
    ||  ch >= '0' && ch <= '9'
    ||  ch == '-')
      continue;
    return false;   // invalid character found
  }
  return true;
}
