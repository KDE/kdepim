/* configdialog.cc              PilotDaemon
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

static const char *configdialog_id = "$Id$";

#include <config.h>
#include "../lib/debug.h"

#include "configdialog.h"

Config::Config() :
	KConfig("pilotdaemon",false,false,"config")
{
}

bool Config::isTransientDevice() const
{
	return readBoolEntry("TransientDevice",false);
}

void Config::setTransientDevice(bool b)
{
	writeEntry("TransientDevice",b);
}

QString Config::device() const
{
	return readEntry("Device");
}

void Config::setDevice(const QString &s)
{
	writeEntry("Device",s);
}

// $Log$
// Revision 1.1.1.1  2001/06/21 19:49:57  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
