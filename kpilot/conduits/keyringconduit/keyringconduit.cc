/* keyringconduit.cc			KPilot
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

#include "keyringconduit.h"

#include <kpassworddialog.h>

#include "options.h"
#include "pilotRecord.h"

#include "keyringhhdataproxy.h"
#include "keyringhhrecord.h"

KeyringConduit::KeyringConduit( KPilotLink *o, const QStringList &a )
 : RecordConduit( o, a, CSL1( "Keyring Conduit" ), CSL1( "Keys-Gtkr.pdb" ) )
{
}

void KeyringConduit::loadSettings()
{
	FUNCTIONSETUP;
}
	
void KeyringConduit::initDataProxies()
{
	FUNCTIONSETUP;
	
	
	// TODO: read password from wallet, or ask user for it.
	/*
	KPasswordDialog dlg( this, KPasswordDialog::ShowKeepPassword );
	dlg.setPrompt( i18n( "Enter your Keyring password" );
	if( !dlg.exec() )
  {
		addSyncLogEntry( i18n( "No password given,  ." ) );
		return; //the user canceled
  }
  */

	QString pass = "Test"; //dlg.password();
	KeyringHHDataProxy *hhDataProxy = new KeyringHHDataProxy( fDatabase );
	
	// TODO: keep user asking for password.
	while( !hhDataProxy->openDatabase( pass ) )
	{
		hhDataProxy->openDatabase( pass );
	}
	
	fHHDataProxy = hhDataProxy;
	
	// Now we know that the password is correct we can create and store the
	// DES-key, which is needed to create records.
	QCA::Initializer init;
	QCA::Hash passHash( "md5" );
	passHash.update( pass.toLatin1() );
	
	// generate the DES keypair (snib = A,B; desKeyData = A,B,A)
	QCA::SymmetricKey key = QCA::SymmetricKey( passHash.final() );
	key.append( key.toByteArray().left( 8 ) );
	fDesKey = QCA::arrayToHex( key.toByteArray() );
	
	// Open the other proxies.
	if( fLocalDatabase )
	{
		KeyringHHDataProxy *backupDataProxy = new KeyringHHDataProxy( fLocalDatabase );
		backupDataProxy->openDatabase( pass );
		
		fBackupDataProxy = backupDataProxy;
	}
	
	//TODO: Open the local database from a file
	
	// Do not keep the password any longer in memory then necessary.
	pass.clear();
}

bool KeyringConduit::equal( Record *pcRec, HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	#warning not implemented
	Q_UNUSED( pcRec );
	Q_UNUSED( hhRec );
	return false;
}

Record* KeyringConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	return new KeyringHHRecord( new PilotRecord( hhRec->pilotRecord() ), fDesKey );
}

HHRecord* KeyringConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	
	// Because the pc data store is of the same format of a hand held datastore
	// we can do this safely. This conduit only deals with KeyringHHRecord objects.
	const KeyringHHRecord *hhRec = static_cast<const KeyringHHRecord*>( pcRec );
	
	return new KeyringHHRecord( new PilotRecord( hhRec->pilotRecord() ), fDesKey );
}

void KeyringConduit::copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	// Make sure we don't do unecessary work.
	if( !from->equal( to ) )
	{
		const KeyringHHRecord *krFrom = static_cast<const KeyringHHRecord*>( from );
		KeyringHHRecord *krTo = static_cast<const KeyringHHRecord*>( to );
		
		if( krTo->name() != krFrom->name() )
		{
			krTo->setName( krFrom->name() );
		}
		
		if( krTo->account() != krFrom->account() )
		{
			krTo->setAccount( krFrom->account() );
		}
		
		if( krTo->password() != krFrom->password() )
		{
			krTo->setPassword( krFrom->password() );
		}
		
		if( krTo->notes() != krFrom->notes() )
		{
			krTo->setNotes( krFrom->notes() );
		}
		
		krTo->setLastChangedDate( QDateTime::currentDateTime() );
	}
}

void KeyringConduit::copy( const HHRecord *from, Record *to  )
{
	// Don't implement things twice, just call the other copy method.
	copy( (Record*) from, static_cast<HHRecord*>( to ) );
}

bool KeyringConduit::createBackupDatabase()
{
	#warning not implemented.
}
