/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kdevice.h"

using namespace KSync;

class Device::DevicePrivate {
public:
  DevicePrivate(){
  }
  QString name;
  QString group;
  QString vendor;
  QString library;
  QString id;
};

Device::Device(const QString &ident, const QString &group,
		 const QString &vendor, const QString &library,
                 const QString &id)
{
  d = new DevicePrivate();
  d->name= ident;
  d->group = group;
  d->vendor = vendor;
  d->library = library;
  d->id = id;
}
Device::Device()
{
  d = new DevicePrivate();
}
Device::Device(const Device &dev )
{
  d = new DevicePrivate();
  d->name = dev.identify();
  d->group = dev.group();
  d->vendor = dev.vendor();
  d->library = dev.library();
  d->id = dev.id();
}
Device &Device::operator=( const Device &dev )
{
    d = new DevicePrivate;
    d->name = dev.d->name;
    d->group = dev.d->group;
    d->vendor = dev.d->vendor;
    d->library = dev.d->library;
    d->id = dev.d->id;

    return *this;
}
Device::~Device()
{
  delete d;
}
QString Device::identify() const
{
  return d->id;
}
QString Device::group() const
{
  return d->group;
}
QString Device::vendor() const
{
  return d->vendor;
}
QString Device::library() const
{
  return d->library;
}
QString Device::id()const
{
    return d->id;
}
bool Device::operator==(const Device &dest ){
  if( identify() == dest.identify() && group() == dest.group() && dest.vendor() == vendor() ){
    return true;
  }else{
    return false;
  }
}
