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

class KDevice::KDevicePrivate {
public:
  KDevicePrivate(){
  }
  QString id;
  QString group;
  QString vendor;
  QString library;
};

KDevice::KDevice(const QString &ident, const QString &group,
		 const QString &vendor, const QString &library)
{
  d = new KDevicePrivate();
  d->id = ident;
  d->group = group;
  d->vendor = vendor;
  d->library = library;
}
KDevice::KDevice()
{
  d = new KDevicePrivate();
}
KDevice::KDevice(const KDevice &dev )
{
  d = new KDevicePrivate();
  d->id = dev.identify();
  d->group = dev.group();
  d->vendor = dev.vendor();
  d->library = dev.library();
}
KDevice::~KDevice()
{
  delete d;
}
QString KDevice::identify() const
{
  return d->id;
}
QString KDevice::group() const
{
  return d->group;
}
QString KDevice::vendor() const
{
  return d->vendor;
}
QString KDevice::library() const
{
  return d->library;
}
bool operator==(const KDevice &orig, const KDevice &dest ){
  if( orig.identify() == dest.identify() && orig.group() == dest.group() && dest.vendor() == orig.vendor() ){
    return true;
  }else{ 
    return false;
  }
}
