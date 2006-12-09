/* SysInfo-factory.cc                      KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This file defines the factory for the SysInfo-conduit plugin.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "pluginfactory.h"

#include "sysinfo-conduit.h"
#include "sysinfo-setup.h"

extern "C"
{

void *init_conduit_sysinfo()
{
	return new ConduitFactory<SysInfoWidgetConfig,SysInfoConduit>;
}

}
