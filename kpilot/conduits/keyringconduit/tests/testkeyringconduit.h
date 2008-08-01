#ifndef TESTKEYRINGCONDUIT_H
#define TESTKEYRINGCONDUIT_H
/* keyringconduit.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "keyringconduit.h"

class TestKeyringProxy;
class IDMapping;

class KPILOT_EXPORT TestKeyringConduit : public KeyringConduit
{
	
public:
	TestKeyringConduit( const QVariantList& args );
	
	/* virtual */ bool initDataProxies();
	
	/**
	 * Perform a hotsync.
	 */
	void hotSync();
	
	IDMapping mapping() const;
	
	TestKeyringProxy* hhProxy() const;
	
	TestKeyringProxy* backupProxy() const;
	
	TestKeyringProxy* pcProxy() const;
	
private:
	// These are kept for testing, we don't have to delete them at deletion as
	// they are passed to KeyringConduit::f*Proxy also, so KeyringConduit will
	// take care of deletion.
	
	TestKeyringProxy* hhDataProxy;
	TestKeyringProxy* backupDataProxy;
	TestKeyringProxy* pcDataProxy;
};

#endif
