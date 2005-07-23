/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
/**
 @file calendarnull.cpp
 A null calendar class with does nothing.

 @author Cornelius Schumacher
*/
#include "calendarnull.h"

using namespace KCal;

CalendarNull::CalendarNull( const QString &timeZoneId )
  : Calendar( timeZoneId )
{}

CalendarNull *CalendarNull::mSelf = 0;

CalendarNull *CalendarNull::self()
{
  if ( !mSelf ) mSelf = new CalendarNull( QString::fromLatin1( "UTC" ) );

  return mSelf;
}
