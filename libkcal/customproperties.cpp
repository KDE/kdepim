/*
    This file is part of libkcal.

    Copyright (c) 2002,2006 David Jarvie <software@astrojar.org.uk>

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
  TQMap<TQCString, TQString>::ConstIterator it;
  for( it = mProperties.begin(); it != mProperties.end(); ++it ) {
    TQMap<TQCString, TQString>::ConstIterator itOther =
      other.mProperties.find( it.key() );

    if ( itOther == other.mProperties.end() ) {
      return false;
    }
    if ( itOther.data() != it.data() ) return false;
  }

  return true;
}

void CustomProperties::setCustomProperty(const TQCString &app, const TQCString &key,
                                         const TQString &value)
{
  if (value.isNull() || key.isEmpty() || app.isEmpty())
    return;
  TQCString property = "X-KDE-" + app + "-" + key;
  if (!checkName(property))
    return;
  mProperties[property] = value;
  customPropertyUpdated();
}

void CustomProperties::removeCustomProperty(const TQCString &app, const TQCString &key)
{
  removeNonKDECustomProperty(TQCString("X-KDE-" + app + "-" + key));
}

TQString CustomProperties::customProperty(const TQCString &app, const TQCString &key) const
{
  return nonKDECustomProperty(TQCString("X-KDE-" + app + "-" + key));
}

void CustomProperties::setNonKDECustomProperty(const TQCString &name, const TQString &value)
{
  if (value.isNull() || !checkName(name))
    return;
  mProperties[name] = value;
  customPropertyUpdated();
}

void CustomProperties::removeNonKDECustomProperty(const TQCString &name)
{
  TQMap<TQCString, TQString>::Iterator it = mProperties.find(name);
  if (it != mProperties.end()) {
    mProperties.remove(it);
    customPropertyUpdated();
  }
}

TQString CustomProperties::nonKDECustomProperty(const TQCString &name) const
{
  TQMap<TQCString, TQString>::ConstIterator it = mProperties.find(name);
  if (it == mProperties.end())
    return TQString::null;
  return it.data();
}

void CustomProperties::setCustomProperties(const TQMap<TQCString, TQString> &properties)
{
  bool changed = false;
  for (TQMap<TQCString, TQString>::ConstIterator it = properties.begin();  it != properties.end();  ++it) {
    // Validate the property name and convert any null string to empty string
    if (checkName(it.key())) {
      mProperties[it.key()] = it.data().isNull() ? TQString("") : it.data();
      changed = true;
    }
  }
  if (changed)
    customPropertyUpdated();
}

TQMap<TQCString, TQString> CustomProperties::customProperties() const
{
  return mProperties;
}

bool CustomProperties::checkName(const TQCString &name)
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
