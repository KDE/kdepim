#ifndef _KPILOT_PILOTLINKVERSION_H
#define _KPILOT_PILOTLINKVERSION_H
/* KPilot
**
** Copyright (C) 2005 by Adriaan de Groot
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <pi-version.h>

/** @file
* Checks the pilot-link version and defines some convenience macros.
* The main point of this file is to complain if you have a version
* of pilot-link before 0.12, which no longer work with KPilot.
*/

#ifndef PILOT_LINK_VERSION
#error "You need at least pilot-link version 0.12.0"
#endif


#define PILOT_LINK_NUMBER	((10000*PILOT_LINK_VERSION) + \
				(100*PILOT_LINK_MAJOR)+PILOT_LINK_MINOR)
#define PILOT_LINK_0_10_0	(1000)
#define PILOT_LINK_0_11_0	(1100)
#define PILOT_LINK_0_11_8	(1108)
#define PILOT_LINK_0_12_0	(1200)
#define PILOT_LINK_0_12_1	(1201)

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
#error "You need at least pilot-link version 0.12.0 for KPilot"
#endif

#define PI_SIZE_T size_t


#endif

