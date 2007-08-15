#ifndef PIKEYRING_H
#define PIKEYRING_H
/* pi-keyring.h			KPilot
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


#include "pilotAppInfo.h"

typedef struct KeyringAppInfo {
	struct CategoryAppInfo category;
} KeyringAppInfo_t;

#if PILOT_LINK_IS(0,12,2)
	int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		const unsigned char *record, size_t len );
	int pack_KeyringAppInfo( const KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
#else  
	int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
	int pack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
#endif


typedef PilotAppInfo<struct KeyringAppInfo, unpack_KeyringAppInfo
	, pack_KeyringAppInfo> PilotKeyringInfo;

#endif
