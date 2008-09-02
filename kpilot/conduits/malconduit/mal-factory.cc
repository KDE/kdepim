/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the factory for the MAL-conduit plugin.
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
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "pluginfactory.h"

#include "mal-conduit.h"
#include "mal-setup.h"

DECLARE_KPILOT_PLUGIN(kpilot_conduit_mal,MALWidgetSetup,MALConduit)



