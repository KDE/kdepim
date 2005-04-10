/* testactions			KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to sync actions.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "syncAction.h"

bool run_modes(bool test, bool local)
{
	bool ok = true;

	kdDebug() << "***\n*** Sync Modes ("
		<< ( test ? "" : "no")
		<< "test, "
		<< ( local ? "" : "no")
		<< "local)\n***\n";


	for (int m = (int)SyncAction::SyncMode::eFastSync;
		m <= (int) SyncAction::SyncMode::eRestore ;
		m++)
	{
		SyncAction::SyncMode mode((SyncAction::SyncMode::Mode)m,test,local);
		kdDebug() << "* " << mode.name() << endl;
		SyncAction::SyncMode mode2(mode.list());
		if (!(mode==mode2)) {
			kdDebug() << "E " << "Modes mismatch [" << mode.name() << "] [" << mode2.name() << "]" << endl;
			ok = false;
		}
	}

	return ok;
}

int main(int argc, char **argv)
{
	if (!run_modes(false,false)) return 1;
	if (!run_modes(false,true)) return 1;
	if (!run_modes(true,false)) return 1;
	if (!run_modes(true,true)) return 1;
	return 0;
}


