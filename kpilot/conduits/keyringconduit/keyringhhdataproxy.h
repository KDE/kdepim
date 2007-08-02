#ifndef KEYRINGHHDATAPROXY_H
#define KEYRINGHHDATAPROXY_H
/* keyringhhdataproxy.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "hhdataproxy.h"

#include <QtCrypto>

class HHRecord;

class KPILOT_EXPORT KeyringHHDataProxy : public HHDataProxy {
public:
	KeyringHHDataProxy( PilotDatabase *db );
	
	virtual ~KeyringHHDataProxy();

protected:
	/**
	 * This function creates a (subclass of) HHRecord for @p rec.
	 */
	virtual HHRecord* createHHRecord( PilotRecord *rec );
	
	virtual bool createDataStore();
	
	static const int MD5_DIGEST_LENGTH = 16;
	static const int MD5_CBLOCK = 64;
	static const int SALT_SIZE = 4;

private: // functions
	QCA::SecureArray getDigest( const QCA::SecureArray &salt
		, const QCA::SecureArray &pass );

private: // members
	PilotRecord *fZeroRecord;
	QCA::SymmetricKey fDesKey;
};
#endif
