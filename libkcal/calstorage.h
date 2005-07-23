/*
    This file is part of libkcal.

    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_CALSTORAGE_H
#define KCAL_CALSTORAGE_H

#include "libkcal_export.h"

namespace KCal {

class Calendar;

/**
  This class provides the interface to the storage of a calendar.
*/
class LIBKCAL_EXPORT CalStorage
{
  public:
    CalStorage( Calendar *calendar )
    {
      mCalendar = calendar;
    }
    virtual ~CalStorage() {}

    Calendar *calendar() const { return mCalendar; }
  
    virtual bool open() = 0;
    virtual bool load() = 0;
    virtual bool save() = 0;
    virtual bool close() = 0;

  private:
    Calendar *mCalendar;

    class Private;
    Private *d;
};

}

#endif
