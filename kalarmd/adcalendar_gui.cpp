/*
    Calendar access for KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "adcalendar_gui.h"


ADCalendarGui::ADCalendarGui(const QString& url, const QString& appname,
                             Type type)
  : ADCalendarBase(url, appname, type),
    mAvailable(false),
    mEnabled(false)
{
  if (type == KORGANIZER)
    loadFile();
  showDialogs(FALSE);
}

ADCalendarGui *ADCalendarGuiFactory::create(const QString& url,
                                            const QString& appname,
                                            ADCalendarBase::Type type)
{
  return new ADCalendarGui(url, appname, type);
}

