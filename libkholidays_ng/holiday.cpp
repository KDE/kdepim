/*
    This file is part of libkholidays.
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "holiday.h"

using namespace KHolidays;

Holiday::Holiday()
{
}

Holiday::Holiday(const Holiday &e)
{
  mDate = e.mDate;
}

Holiday::~Holiday()
{
}

Holiday *Holiday::clone()
{
  return new Holiday(*this);
}

bool Holiday::operator==( const Holiday& e2 ) const
{
    return
      static_cast<const Holiday&>(*this) == static_cast<const Holiday&>(e2) &&
      date() == e2.date() &&
      name() == e2.name();
}

void Holiday::setName( const QString &name )
{
  mName = name;
}

QString Holiday::name() const
{
  return mName;
}

void Holiday::setDate( const QDate &date )
{
  mDate = date;
}

QDate Holiday::date() const
{
  return mDate;
}

QString Holiday::dateStr() const
{
  return KGlobal::locale()->formatDate( mDate );
}

QString Holiday::dateStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatDate( mDate, shortfmt );
}
