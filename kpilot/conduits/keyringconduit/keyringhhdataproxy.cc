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

#include <pi-util.h>

#include <QtCrypto>

#include "options.h"
#include "pilot.h"
#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotRecord.h"

#include "keyringhhrecord.h"

KeyringHHDataProxy::KeyringHHDataProxy( PilotDatabase *db )
	: HHDataProxy( db ), fZeroRecord( 0l )
{
	FUNCTIONSETUP;
	
	// Hash-key record
	if( fDatabase && fDatabase->isOpen() )
	{
		fZeroRecord = fDatabase->readRecordByIndex( 0 );
	}
}

KeyringHHDataProxy::KeyringHHDataProxy( const QString &dbPath )
	: HHDataProxy( 0l ), fZeroRecord( 0l )
{
	FUNCTIONSETUP;
	
	if( dbPath.right( 4 ) == CSL1( ".pdb" ) )
	{
		fDatabase = new PilotLocalDatabase( dbPath.left( dbPath.length() - 4 ) );
	}
	else
	{
		DEBUGKPILOT << "invalid file name.";
	}
}

KeyringHHDataProxy::~KeyringHHDataProxy()
{
	if( fZeroRecord )
	{
		delete fZeroRecord;
		fZeroRecord = 0l;
	}
}

bool KeyringHHDataProxy::openDatabase( const QString &pass )
{
	FUNCTIONSETUP;
	
	QCA::Initializer init;
	
	QCA::SecureArray passArray = pass.toLatin1();
	
	QCA::Hash passHash( "md5" );
	passHash.update( passArray );
	
	// generate the DES keypair (snib = A,B; desKeyData = A,B,A)
	QCA::SymmetricKey key = QCA::SymmetricKey( passHash.final() );
	key.append( key.toByteArray().left( 8 ) );
	fDesKey = QCA::arrayToHex( key.toByteArray() );
	
	if( fDatabase && fZeroRecord )
	{
		QCA::SecureArray recordZero( fZeroRecord->data() );
		QCA::SecureArray hash = getDigest( recordZero, passArray );
		
		// The salt and password hash are stored in recordZero.
		if( recordZero.toByteArray().contains( hash.toByteArray() ) )
		{
			qDebug() << "Password correct!";
			
			// Pass is correct and DES encryption key known, load the records.
			loadAllRecords();
			
			// For now remove the zero record from the record list.
			fRecords.remove( QString::number( fZeroRecord->id() ) );
			
			return true;
		}
		else
		{
			qDebug() << "Password incorrect!";
			return false;
		}
	}
	else
	{
		// There is no data base so most probably createDataStore() will be called.
		// But for that we need a hash that is stored in the zerorecord and by that
		// time this class doesn't know the password anymore. We also don't want to
		// store it plain text so we create a salt and a hash right here and store
		// that so that it can be used by createDataStore().
		QCA::SecureArray salt = QCA::Random::randomArray( SALT_SIZE );
		QCA::SecureArray saltedHashData( salt );
		saltedHashData.append( passArray );
		
		QCA::Hash hash( "md5" );
		hash.update( saltedHashData );
		
		QByteArray saltedHash;
		saltedHash.append( salt.toByteArray() );
		saltedHash.append( hash.final().toByteArray() );
		
		fSaltedHash = QCA::arrayToHex( saltedHash );
		
		return false;
	}
	
	return false;
}

HHRecord* KeyringHHDataProxy::createHHRecord( PilotRecord *rec )
{
	FUNCTIONSETUP;
	
	return new KeyringHHRecord( rec, fDesKey );
}

bool KeyringHHDataProxy::createDataStore()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		return false;
	}
	
	// No usefull data for the zero record.
	if( fDesKey.isEmpty() )
	{
		return false;
	}
	
	if( !fDatabase->isOpen() )
	{
		// File did not exist
		long creator = pi_mktag( 'G', 'k','t','r' );
		long type = pi_mktag( 'G', 'k','y','r' );
		fDatabase->createDatabase( creator, type, 0, 0, 4 );
		
		QByteArray saltedHash = QCA::hexToArray( fSaltedHash );
		
		pi_buffer_t *buf = pi_buffer_new( saltedHash.size() );
		memcpy( buf->data, (unsigned char*) saltedHash.data(), saltedHash.size() );
		
		fZeroRecord = new PilotRecord( buf, 0, 0, 0);
		fZeroRecord->setSecret();
		
		fDatabase->writeRecord( fZeroRecord );
		
		// Create a record to show KPilot was there =:)
		KeyringHHRecord *rec = new KeyringHHRecord( CSL1( "KPilot" )
			, CSL1( "KPilot" ), CSL1( "KPilot" )
			, CSL1( "This database is created with KPilot."
					"\nThanks for using kpilot!" )
			, fDesKey );
		fDatabase->writeRecord( rec->pilotRecord() );
		
		return true;
	}
	else
	{
		// There seems to be a database already. But this shouldn't happen i think.
		return true;
	}
}
	
QCA::SecureArray KeyringHHDataProxy::getDigest(
	const QCA::SecureArray &recordZero, const QCA::SecureArray &pass )
{
	FUNCTIONSETUP;
	
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
