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

int main(int argc, char **argv)
{
#ifdef DEBUG
	debug_level = 1;
#endif
	PilotAppInfoBase info( 0L );


	kdDebug() << "### testconstants\n#" << endl;
	kdDebug() << "# Sizes of structures\n#" << endl;
	kdDebug() << "#     AppInfoBase: " << sizeof(PilotAppInfoBase) << endl;
	kdDebug() << "#    CategoryInfo: " << sizeof(info.categoryInfo()) << endl;
	kdDebug() << "#    CategoryInfo: " << sizeof(*info.categoryInfo()) << endl;
	kdDebug() << "#  Category names: " << sizeof(info.categoryInfo()->name) << endl;
	kdDebug() << "# Single category: " << sizeof(info.categoryInfo()->name[0]) << endl;

	kdDebug() << "#\n# Sanity checking structure sizes\n#" << endl;
	if ( sizeof(info.categoryInfo()->name[0]) != Pilot::CATEGORY_SIZE )
	{
		kdDebug() << "! Category names are not 16 bytes." << endl;
		return 1;
	}
	if ( sizeof(info.categoryInfo()->name) / sizeof(info.categoryInfo()->name[0]) != Pilot::CATEGORY_COUNT )
	{
		kdDebug() << "! There are not " << Pilot::CATEGORY_COUNT << " categories available." << endl;
		return 1;
	}

	kdDebug() << "# OK.\n" << endl;
	return 0;
}


