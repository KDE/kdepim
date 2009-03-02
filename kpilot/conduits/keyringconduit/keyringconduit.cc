/* keyringconduit.cc			KPilot
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

#include <QApplication>

#include <kpassworddialog.h>
#include <kwallet.h>

#include "options.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"

#include "keyringhhdataproxy.h"
#include "keyringhhrecord.h"
#include "keyringsettings.h"

using namespace KWallet;

KeyringConduit::KeyringConduit( KPilotLink *o, const QVariantList &a )
 : RecordConduit( o, a, CSL1( "Keys-Gtkr" ), CSL1( "Keyring Conduit" ) )
{
}

void KeyringConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	fPcDatastoreUrl = KeyringConduitSettings::databaseUrl();
	
	if( KeyringConduitSettings::passwordSetting() == KeyringConduitSettings::Wallet )
	{
		fAskPass = false;
	}
	else
	{
		fAskPass = true;
	}
}
	
bool KeyringConduit::initDataProxies()
{
	FUNCTIONSETUP;

	QCA::Initializer init;
	if(!QCA::isSupported("tripledes-cbc")) {
		WARNINGKPILOT << "Error: Triple DES not supported! Unable to continue.";
		addSyncLogEntry(i18n("Error: Triple DES not supported. Unable to continue."));
		return false;
	}
	
	KeyringHHDataProxy *hhDataProxy = new KeyringHHDataProxy( fDatabase );
	
	QString pass;
	
	if( fAskPass )
	{
		pass = askPassword();
		
		// User pressed cancel
		if( pass.isNull() )
		{
			return false;
		}
		
		while( !hhDataProxy->openDatabase( pass ) )
		{
			// User pressed cancel
			if( pass.isNull() )
			{
				return false;
			}
			
			DEBUGKPILOT << "Trying password: " << pass;
			
			addSyncLogEntry( i18n( "Password invalid." ) );
			pass = askPassword();
		}
	}
	else
	{
		// Read pass from wallet.
		WId window(0L);
	
		Wallet *wallet = Wallet::openWallet( Wallet::LocalWallet(), window );
		if ( ! wallet )
		{
			WARNINGKPILOT << "Error. Could not open wallet!";
			addSyncLogEntry( i18n( "Error: Could not open wallet." ) );
			return false;
		}
		
		QString passwordFolder = Wallet::PasswordFolder();
		if ( wallet->hasFolder( passwordFolder ) )
		{
			wallet->setFolder( passwordFolder );
			
			wallet->readPassword( CSL1( "kpilot-keyring" ), pass );
			Wallet::disconnectApplication( Wallet::LocalWallet(), CSL1( "KPilot" ) );
			
			if( !hhDataProxy->openDatabase( pass ) )
			{
				addSyncLogEntry( i18n( "Password invalid. Update your password in "
					"the settings dialog." ) );
			}
		}
	}
	
	fHHDataProxy = hhDataProxy;
	
	// Now we know that the password is correct we can create and store the
	// DES-key, which is needed to create records.
	QCA::Hash passHash( "md5" );
	passHash.update( pass.toLatin1() );
	
	// generate the DES keypair (snib = A,B; desKeyData = A,B,A)
	QCA::SymmetricKey key = QCA::SymmetricKey( passHash.final() );
	key.append( key.toByteArray().left( 8 ) );
	fDesKey = QCA::arrayToHex( key.toByteArray() );
	
	// Open the other proxies.
	if( fLocalDatabase )
	{
		DEBUGKPILOT << "Local backup: " << fLocalDatabase->dbPathName();
		
		KeyringHHDataProxy *backupDataProxy = new KeyringHHDataProxy( fLocalDatabase );
		backupDataProxy->openDatabase( pass );
		
		fBackupDataProxy = backupDataProxy;
	}
	
	// We cannot create a keyring datastore if we don't have a url.
	if( fPcDatastoreUrl.isEmpty() )
	{
		return false;
	}
	
	// TODO: For now we assume that the pc datastore has the same pass as the
	//       handheld datastore.
	DEBUGKPILOT << "Local keyring file: " << fPcDatastoreUrl;
	KeyringHHDataProxy *pcDataProxy = new KeyringHHDataProxy( fPcDatastoreUrl );
	pcDataProxy->openDatabase( pass );
	
	fPCDataProxy = pcDataProxy;
	
	// Do not keep the password any longer in memory then necessary.
	pass.clear();
	
	// We added a record to the newly created database so load it.
	fPCDataProxy->loadAllRecords();
	
	return true;
}

QString KeyringConduit::askPassword() const
{
	FUNCTIONSETUP;
	
	KPasswordDialog dlg( 0, KPasswordDialog::NoFlags );
	dlg.setPrompt( i18n( "Enter your Keyring password" ) );
	if( !dlg.exec() )
	{
		return QString();
	}
	else
	{
		return dlg.password();
	}
}

bool KeyringConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	
	if( !pcRec || !hhRec )
	{
		return false;
	}
	
	return hhRec->equal( static_cast<const HHRecord*>( pcRec )  );
}

Record* KeyringConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	// TODO: Replace QString() by the right category name.
	return new KeyringHHRecord( new PilotRecord( hhRec->pilotRecord() )
		, hhRec->category(), fDesKey );
}

HHRecord* KeyringConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	
	// Because the pc data store is of the same format of a hand held datastore
	// we can do this safely. This conduit only deals with KeyringHHRecord objects.
	const KeyringHHRecord *hhRec = static_cast<const KeyringHHRecord*>( pcRec );
	
	return new KeyringHHRecord( new PilotRecord( hhRec->pilotRecord() )
		, hhRec->category(), fDesKey );
}

void KeyringConduit::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	const KeyringHHRecord *krFrom = static_cast<const KeyringHHRecord*>( from );
	
	// Make sure we don't do unnecessary work.
	if( !krFrom->equal( to ) )
	{
		KeyringHHRecord *krTo = static_cast<KeyringHHRecord*>( to );
		
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

void KeyringConduit::_copy( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;
	
	// Don't implement things twice, just call the other copy method.
	_copy( (Record*) from, static_cast<HHRecord*>( to ) );
}
