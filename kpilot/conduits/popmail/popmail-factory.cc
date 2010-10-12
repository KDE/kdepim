/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org>
**
** This file defines the factory for the popmail-conduit plugin.
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


#include "setupDialog.h"
#include "popmail-conduit.h"
#include "pluginfactory.h"


extern "C"
{

void *init_conduit_popmail()
{
	return new ConduitFactory<PopMailWidgetConfig,PopMailConduit>;
}

}

