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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "customproperties.h"

#include <kdebug.h>

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
  QMap<QCString, QString>::ConstIterator it;
  for( it = mProperties.begin(); it != mProperties.end(); ++it ) {
    QMap<QCString, QString>::ConstIterator itOther =
      other.mProperties.find( it.key() );

    if ( itOther == other.mProperties.end() ) {
      return false;
    }
    if ( itOther.data() != it.data() ) return false;
  }

  return true;
}

void CustomProperties::setCustomProperty(const QCString &app, const QCString &key,
                                         const QString &value)
{
  if (value.isNull() || key.isEmpty() || app.isEmpty())
    return;
  QCString property = "X-KDE-" + app + "-" + key;
  if (!checkName(property))
    return;
  mProperties[property] = value;
}

void CustomProperties::removeCustomProperty(const QCString &app, const QCString &key)
{
  removeNonKDECustomProperty(QCString("X-KDE-" + app + "-" + key));
}

QString CustomProperties::customProperty(const QCString &app, const QCString &key) const
{
  return nonKDECustomProperty(QCString("X-KDE-" + app + "-" + key));
}

void CustomProperties::setNonKDECustomProperty(const QCString &name, const QString &value)
{
  if (value.isNull() || !checkName(name))
    return;
  mProperties[name] = value;
}

void CustomProperties::removeNonKDECustomProperty(const QCString &name)
{
  QMap<QCString, QString>::Iterator it = mProperties.find(name);
  if (it != mProperties.end())
    mProperties.remove(it);
}

QString CustomProperties::nonKDECustomProperty(const QCString &name) const
{
  QMap<QCString, QString>::ConstIterator it = mProperties.find(name);
  if (it == mProperties.end())
    return QString::null;
  return it.data();
}

void CustomProperties::setCustomProperties(const QMap<QCString, QString> &properties)
{
  for (QMap<QCString, QString>::ConstIterator it = properties.begin();  it != properties.end();  ++it) {
    // Validate the property name and convert any null string to empty string
    if (checkName(it.key())) {
      mProperties[it.key()] = it.data().isNull() ? QString("") : it.data();
    }
  }
}

QMap<QCString, QString> CustomProperties::customProperties() const
{
  return mProperties;
}

bool CustomProperties::checkName(const QCString &name)
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
