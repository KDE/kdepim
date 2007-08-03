/* keyringhhdataproxy.cc			KPilot
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

#include "keyringhhdataproxy.h"

#include "options.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"

#include "keyringhhrecord.h"

KeyringHHDataProxy::KeyringHHDataProxy( PilotDatabase *db ) : HHDataProxy( db )
{
	FUNCTIONSETUP;
	
	// Hash-key record
	fZeroRecord = fDatabase->readRecordByIndex( 0 );
	
	// salt should be put in a QCA::SecureArray, but it gave me segfaults.
	QCA::Initializer init;
	QCA::SecureArray recordZero( fZeroRecord->data() );
	QCA::SecureArray pass( "test" );
	QCA::SecureArray hash = getDigest( recordZero, pass );
	
	// The salt and password hash are stored in recordZero.
	if( recordZero.toByteArray().contains( hash.toByteArray() ) )
	{
		DEBUGKPILOT << "Password correct!";
		
		QCA::Hash passHash( "md5" );
		passHash.update( pass );
		
		// generate the DES keypair (snib = A,B; desKeyData = A,B,A)
		QCA::SymmetricKey key = QCA::SymmetricKey( passHash.final() );
		key.append( key.toByteArray().left( 8 ) );
		fDesKey = QCA::arrayToHex( key.toByteArray() );
		
		// Pass is correct and DES encryption key known, load the records.
		loadAllRecords();
		
		// For now remove the zero record from the record list.
		fRecords.remove( QString::number( fZeroRecord->id() ) );
	}
	else
	{
		DEBUGKPILOT << "Password incorrect!";
	}
}

KeyringHHDataProxy::~KeyringHHDataProxy()
{
	delete fZeroRecord;
}

HHRecord* KeyringHHDataProxy::createHHRecord( PilotRecord *rec )
{
	FUNCTIONSETUP;
	
	return new KeyringHHRecord( rec, fDesKey );
}

bool KeyringHHDataProxy::createDataStore()
{
	#warning not implemented
	return false;
}
	
QCA::SecureArray KeyringHHDataProxy::getDigest(
	const QCA::SecureArray &recordZero, const QCA::SecureArray &pass )
{
	QCA::SecureArray msg( SALT_SIZE, 0 );
	
	// 32 bit salt -> first four byte of recordZero are the salt.
	for( int i = 0; i < SALT_SIZE; i++ )
	{
		msg[i] = recordZero[i];
	}

	// S A L T P A S S W O R D 0 ...
	msg.append( pass );
	msg.append( QCA::SecureArray( MD5_CBLOCK - SALT_SIZE - pass.size(), 0 ) );
	
	QCA::Hash hash1( "md5" );
	hash1.update( msg );
	
	return QCA::SecureArray( hash1.final() );
}
