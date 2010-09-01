/*
    This file is part of KOrganizer.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_COREHELPER_H
#define KORG_COREHELPER_H

#include <tqstring.h>
#include <tqdatetime.h>
#include <tqcolor.h>
#include "printplugin.h"

class KCalendarSytstem;

namespace KOrg {

class CoreHelper
{
  public:
    CoreHelper() {}
    virtual ~CoreHelper() {}

    virtual TQColor textColor( const TQColor &bgColor ) = 0;
    virtual TQColor categoryColor( const TQStringList &cats ) = 0;
    virtual TQString holidayString( const TQDate &dt ) = 0;
    virtual TQTime dayStart() = 0;
    virtual const KCalendarSystem *calendarSystem() = 0;
    virtual KOrg::PrintPlugin::List loadPrintPlugins() = 0;
    virtual bool isWorkingDay( const TQDate &dt ) = 0;
};

}
#endif
