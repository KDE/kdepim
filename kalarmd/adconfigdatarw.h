/*
    KDE Alarm Daemon.

    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

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
#ifndef ADCONFIGDATARW_H
#define ADCONFIGDATARW_H

#include <ksimpleconfig.h>

#include "calclient.h"
#include "adcalendar.h"
#include "adconfigdatabase.h"

// Provides read-write access to the Alarm Daemon config files
class ADConfigDataRW : public ADConfigDataBase
{
  public:
    ADConfigDataRW()  : ADConfigDataBase(true) { }
    virtual ~ADConfigDataRW() {}
    void readDaemonData(bool sessionStarting);
    void writeConfigClient(const QString& appName, const ClientInfo&);
    void writeConfigClientGui(const QString& appName, const QString& dcopObject);
    void addConfigClient(KSimpleConfig&, const QString& appName, const QString& key);
    void writeConfigCalendar(const QString& appName, const ADCalendarBase*);
    virtual void   deleteConfigCalendar(const ADCalendarBase*);

    typedef QMap<QString, QString> GuiMap;  // maps GUI client names against DCOP object names

    GuiMap         mGuis;                // client GUI application names and data
};

#endif
