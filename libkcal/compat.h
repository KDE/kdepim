/*
    This file is part of libkcal.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_COMPAT_H
#define KCAL_COMPAT_H

#include <qstring.h>
#include <qdatetime.h>

namespace KCal {

class Incidence;
class Compat;

class CompatFactory
{
  public:
    static Compat *createCompat( const QString &productId );
};

class Compat
{
  public:
    Compat() {};
    virtual ~Compat() {};

    virtual void fixRecurrence( Incidence * );
    virtual void fixEmptySummary( Incidence * );
    virtual void fixFloatingEnd( QDate & ) {}
    virtual bool useTimeZoneShift() { return true; }
};

class CompatPre32 : public Compat
{
  public:
    virtual void fixRecurrence( Incidence * );
};

class CompatPre31 : public CompatPre32
{
  public:
    virtual void fixFloatingEnd( QDate & );
};

class Compat32PrereleaseVersions : public Compat
{
  public:
    virtual bool useTimeZoneShift() { return false; }
};

}

#endif
