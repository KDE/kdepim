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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
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


	for (int m = (int)SyncAction::SyncMode::eHotSync;
		m <= (int) SyncAction::SyncMode::eRestore ;
		m++)
	{
		SyncAction::SyncMode mode((SyncAction::SyncMode::Mode)m,test,local);
		kdDebug() << "* " << mode.name() << endl;
		SyncAction::SyncMode mode2(mode.list());
		if (!(mode==mode2)) {
			kdDebug() << "E " << "Modes mismatch [" << mode.name() << "] ["
				<< mode2.name() << "]" << endl;
			ok = false;
		}
	}

	return ok;
}

bool single_mode(int m, bool test, bool local)
{
	SyncAction::SyncMode mode((SyncAction::SyncMode::Mode)m,test,local);

	kdDebug() << "* " << m << " " << test << " " << local << endl;

	if ((mode.mode() == m) && (mode.isTest() == test) && (mode.isLocal() == local))
	{
		return true;
	}
	else
	{
		kdDebug() << "E " << "Modes mismatch " << m << " " << test << " " << local
			<< "[" << mode.name() << "]" << endl;
		return false;
	}
}

int main(int argc, char **argv)
{
	if (!run_modes(false,false)) return 1;
	if (!run_modes(false,true)) return 1;
	if (!run_modes(true,false)) return 1;
	if (!run_modes(true,true)) return 1;

	kdDebug() << "***\n*** Sync Modes - misc\n***\n";
	if (!single_mode(3,false,false)) return 1;
	if (!single_mode(1,true,true)) return 1;

	return 0;
}


