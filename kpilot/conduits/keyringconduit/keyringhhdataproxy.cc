/* keyringhhdataproxy.cc			KPilot
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

#include <pi-util.h>

#include <QtCrypto>

#include "options.h"
#include "pilot.h"
#include "pilotDatabase.h"
#include "pilotAppInfo.h"
#include "pilotLocalDatabase.h"
#include "pilotRecord.h"

#include "keyringhhrecord.h"
#include "pi-keyring.h"

#include "keyringhhdataproxy.h"

KeyringHHDataProxy::KeyringHHDataProxy( PilotDatabase *db )
	: HHDataProxy( db ), fZeroRecord( 0l ), fOwner( false )
{
	FUNCTIONSETUP;
	
	// Hash-key record
	if( fDatabase && fDatabase->isOpen() )
	{
		DEBUGKPILOT << "Database open, reading zeroRecord.";
		fZeroRecord = fDatabase->readRecordByIndex( 0 );
	}
}

KeyringHHDataProxy::KeyringHHDataProxy( const QString &dbPath )
	: HHDataProxy( 0l ), fZeroRecord( 0l ), fOwner( true )
{
	FUNCTIONSETUP;
	
	if( dbPath.right( 4 ) == CSL1( ".pdb" ) )
	{
		fDatabase = new PilotLocalDatabase( dbPath.left( dbPath.length() - 4 ) );
		if( fDatabase && fDatabase->isOpen() )
		{
			fZeroRecord = fDatabase->readRecordByIndex( 0 );
		}
	}
	else
	{
		DEBUGKPILOT << "invalid file name.";
	}
}

KeyringHHDataProxy::~KeyringHHDataProxy()
{
	FUNCTIONSETUP;
	
	if( fOwner )
	{
		// We created the database ourself so we should delete it.
		DEBUGKPILOT << "Saving " << fDatabase->recordCount() << " records.";
		KPILOT_DELETE( fDatabase );
	}
	
	KPILOT_DELETE( fZeroRecord );
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
			DEBUGKPILOT << "Password correct!";
			
			// Pass is correct and DES encryption key known, load the records.
			loadAllRecords();
			
			// For now remove the zero record from the record list.
			fRecords.remove( QString::number( fZeroRecord->id() ) );
			
			// Startcount is set by loadAllRecords, but because the zero record doesn't
			// count we reset the start count here.
			fCounter.setStartCount( fRecords.count() );
			
			return true;
		}
		else
		{
			DEBUGKPILOT << "Password incorrect!";
			return false;
		}
	}
	else
	{
		DEBUGKPILOT << "fDatabase: " << fDatabase << " fZeroRecord: " << fZeroRecord;
		// There is no data base so most probably createDataStore() will be called.
		// But for that we need a hash that is stored in the zerorecord and by that
		// time this class doesn't know the password anymore. We also don't want to
		// store it plain text so we create a salt and a hash right here and store
		// that so that it can be used by createDataStore().
		QCA::SecureArray salt = QCA::Random::randomArray( SALT_SIZE );
		QCA::SecureArray saltedHashData( salt );
		saltedHashData.append( passArray );
		saltedHashData.append(
			QCA::SecureArray( MD5_CBLOCK - SALT_SIZE - passArray.size(), 0 ) );
		
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
	
	return new KeyringHHRecord( rec, "Unfiled", fDesKey );
}

void KeyringHHDataProxy::setCategory( Record* rec, const QString& category )
{
	//FIXME: Needs implementation
}

void KeyringHHDataProxy::addCategory( Record* rec, const QString& category )
{
	//FIXME: Needs implementation
}

bool KeyringHHDataProxy::createDataStore()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		DEBUGKPILOT << "No database object.";
		return false;
	}
	
	// No useful data for the zero record.
	if( fDesKey.isEmpty() )
	{
		DEBUGKPILOT << "No deskey object.";
		return false;
	}
	
	if( !fDatabase->isOpen() )
	{
		DEBUGKPILOT << "Creating database: " << fDatabase->dbPathName();
		
		// File did not exist
		long creator = pi_mktag( 'G', 'k','t','r' );
		long type = pi_mktag( 'G', 'k','y','r' );
		fDatabase->createDatabase( creator, type, 0, 0, 4 );
		
		PilotKeyringInfo appInfo;
		appInfo.setCategoryName( 0, CSL1( "Unfiled" ) );
		appInfo.setCategoryName( 1, CSL1( "Banking" ) );
		appInfo.setCategoryName( 2, CSL1( "Computer" ) );
		appInfo.setCategoryName( 3, CSL1( "Phone" ) );
		// rather than adding the 5th default category, add our first record
		// below and set it to this new one.  end result is the same, but this also
		// makes sure that our HHRecord is able to update the appInfo block when
		// it sets the categoryName to a new one....
		// appInfo.setCategoryName( 4, CSL1( "Web" ) );
		
		QByteArray saltedHash = QCA::hexToArray( fSaltedHash );
		
		pi_buffer_t *buf = pi_buffer_new( saltedHash.size() );
		buf->used = saltedHash.size();
		memcpy( buf->data, (unsigned char*) saltedHash.data(), saltedHash.size() );
		
		fZeroRecord = new PilotRecord( buf, 0, 0, 0);
		fZeroRecord->setSecret();
		
		fDatabase->writeRecord( fZeroRecord );
		
		// create a first record and thank our users (2 birds, one stone)  =:).  Note--
		// we also add our 5th category by setting the category for this record
		// to a new one...
		KeyringHHRecord * fFirstRecord = 
			new KeyringHHRecord( i18n("KPilot cares"), i18n("KDE"), "",
					i18n("Thanks for using KPilot!"), fDesKey);
		
		// FIXME: This is deprecated code.
		//fFirstRecord->setCategoryNames(QStringList() << CSL1("Web"));
		
		fDatabase->writeRecord( fFirstRecord->pilotRecord() );
		
		// now write the appInfo block, which includes the 4 explicitly-added categories
		// and the 5th one set via setCategoryNames()
		appInfo.writeTo( fDatabase );
		
		loadAllRecords();
		
		// For now remove the zero record from the record list.
		fRecords.remove( QString::number( fZeroRecord->id() ) );
		
		// Startcount is set by loadAllRecords, but because the zero record doesn't
		// count we reset the start count here.
		fCounter.setStartCount( fRecords.count() );
		
		return true;
	}
	else
	{
		DEBUGKPILOT << "Database already open.";
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

PilotAppInfoBase* KeyringHHDataProxy::readAppInfo()
{
	if( fDatabase && fDatabase->isOpen() )
	{
		PilotKeyringInfo* appInfo = new PilotKeyringInfo( fDatabase );
		
		return appInfo;
	}

	return 0;
}
