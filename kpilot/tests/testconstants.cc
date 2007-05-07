/* testconstants KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Checks that various data structures are sized properly.
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

#include "options.h"

#include "pilot.h"
#include "pilotAppInfo.h"

#include <pi-appinfo.h>

int main(int, char **)
{
#ifdef DEBUG
	debug_level = 1;
#endif
	PilotAppInfoBase info;


	DEBUGKPILOT << "### testconstants\n#" << endl;
	DEBUGKPILOT << "# Sizes of structures\n#" << endl;
	DEBUGKPILOT << "#     AppInfoBase: " << sizeof(PilotAppInfoBase) << endl;
	DEBUGKPILOT << "#    CategoryInfo: " << sizeof(info.categoryInfo()) << endl;
	DEBUGKPILOT << "#    CategoryInfo: " << sizeof(*info.categoryInfo()) << endl;
	DEBUGKPILOT << "#  Category names: " << sizeof(info.categoryInfo()->name) << endl;
	DEBUGKPILOT << "# Single category: " << sizeof(info.categoryInfo()->name[0]) << endl;

	DEBUGKPILOT << "#\n# Sanity checking structure sizes\n#" << endl;
	if ( sizeof(info.categoryInfo()->name[0]) != Pilot::CATEGORY_SIZE )
	{
		WARNINGKPILOT << "! Category names are not 16 bytes." << endl;
		return 1;
	}
	if ( sizeof(info.categoryInfo()->name) / sizeof(info.categoryInfo()->name[0]) != Pilot::CATEGORY_COUNT )
	{
		WARNINGKPILOT << "! There are not " << Pilot::CATEGORY_COUNT << " categories available." << endl;
		return 1;
	}

	DEBUGKPILOT << "# OK.\n" << endl;
	return 0;
}


