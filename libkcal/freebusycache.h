/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_FREEBUSYCACHE_H
#define KCAL_FREEBUSYCACHE_H

#include <QString>

namespace KCal {

class FreeBusy;

class FreeBusyCache
{
  public:
	virtual ~FreeBusyCache(){}
	/**
      Save freebusy information belonging to an email.
    */
    virtual bool saveFreeBusy( FreeBusy *freebusy, const Person &person ) = 0;
//    virtual bool saveFreeBusy( FreeBusy *, const QString &email ) = 0;

    /**
      Load freebusy information belonging to an email.
    */
    virtual FreeBusy *loadFreeBusy( const QString &email ) = 0;
};

}

#endif
