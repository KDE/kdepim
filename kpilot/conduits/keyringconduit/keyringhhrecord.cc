/* keyringhhrecord.cc			KPilot
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

#include "keyringhhrecord.h"

#include "options.h"
#include "pilotRecord.h"

KeyringHHRecord::KeyringHHRecord( PilotRecord *rec, const QString &category
                                , const QString &key )
	: HHRecord( rec, category ), fKey( key )
{
	fName = QString( fRecord->data() );
}

KeyringHHRecord::KeyringHHRecord( const QString &name
                                , const QString &account
                                , const QString &password
                                , const QString &notes
                                , const QString &key )
	: HHRecord( 0L, QString() ), fKey( key ), fName( name )
{
	KeyringHHRecordBase data;
	data.account = account;
	data.password = password;
	data.notes = notes;
	data.lastChanged = QDateTime::currentDateTime();
	
	pi_buffer_t *buf = pi_buffer_new( QString( "" ).size() );
	Pilot::toPilot( QString(""), buf->data, 0 );
		
	fRecord = new PilotRecord( buf, 0, 0, 0);
	fRecord->setCategory( 0 );
	fRecord->setDeleted( false );
	fRecord->setSecret( false );
	fRecord->setArchived( false );
	fCategory = "Unfiled";
	
	pack( data );
}

QString KeyringHHRecord::description() const
{
	return name();
}

bool KeyringHHRecord::equal( const HHRecord* other ) const
{
	FUNCTIONSETUP;
	
	// Do not compare last synced date, that's unnecessary.
	const KeyringHHRecord *krOther = static_cast<const KeyringHHRecord*>( other );
	
	bool equal = true;
	
	KeyringHHRecordBase data = unpack();
	
	equal = equal && ( fName == krOther->name() );
	equal = equal && ( data.account == krOther->account() );
	equal = equal && ( data.password == krOther->password() );
	equal = equal && ( data.notes == krOther->notes() );
	
	return equal;
}

QString KeyringHHRecord::name() const
{
	FUNCTIONSETUP;
	
	return fName;
}

QString KeyringHHRecord::account() const
{
	FUNCTIONSETUP;
	
	return unpack().account;
}

QString KeyringHHRecord::password() const
{
	FUNCTIONSETUP;
	
	return unpack().password;
}

QString KeyringHHRecord::notes() const
{
	FUNCTIONSETUP;
	
	return unpack().notes;
}

QDateTime KeyringHHRecord::lastChangedDate() const
{
	FUNCTIONSETUP;
	
	return unpack().lastChanged;
}

void KeyringHHRecord::setName( const QString &name )
{
	KeyringHHRecordBase data = unpack();
	fName = name;
	pack( data );
}

void KeyringHHRecord::setAccount( const QString &account  )
{
	KeyringHHRecordBase data = unpack();
	data.account = account;
	pack( data );
}

void KeyringHHRecord::setPassword( const QString &password  )
{
	KeyringHHRecordBase data = unpack();
	data.password = password;
	pack( data );
}

void KeyringHHRecord::setNotes( const QString &notes  )
{
	KeyringHHRecordBase data = unpack();
	data.notes = notes;
	pack( data );
}

void KeyringHHRecord::setLastChangedDate( const QDateTime &lastChangedDate )
{
	KeyringHHRecordBase data = unpack();
	data.lastChanged = lastChangedDate;
	pack( data );
}

KeyringHHRecordBase KeyringHHRecord::unpack() const
{
	FUNCTIONSETUP;
	
	KeyringHHRecordBase data;

	int n = fName.size() + 1; // String length + zero.
	int size = fRecord->size();

	QCA::Initializer init;
	QCA::SecureArray encryptedData(
		QByteArray( fRecord->data(), size ).right( size - n) );
	
	// Encrypted data is in fRecord->data()[n..size]
	QCA::SymmetricKey sKey( QCA::hexToArray( fKey ) );
	
	if( !QCA::isSupported( "tripledes-ecb" ) )
	{
		WARNINGKPILOT << "ERROR: tripledes not supported! Unable to continue.";
		return data;
	}

	QCA::Cipher cipher( "tripledes", QCA::Cipher::ECB
		, QCA::Cipher::NoPadding , QCA::Decode, sKey );
	QCA::SecureArray result = cipher.update( encryptedData );
	cipher.final();
	
	int pos = 0;
	
	QByteArray dateArray;
	
	for( int i = 0; i < result.size(); i++ )
	{
		if( result[i] )
		{
			switch( pos )
			{
				case 0:
					data.account.append( QChar( result[i] ) );
					break;
				case 1:
					data.password.append( QChar( result[i] ) );
					break;
				case 2:
					data.notes.append( QChar( result[i] ) );
					break;
				case 3:
					dateArray.append( result[i] );
					break;
				default:
					break;
			}
		}
		else
		{
			// 0 read, n-ext item of the record.
			pos++;
		}
	}
	
	// Copied from keyring.c (j-pilot conduit)
	unsigned short packed_date = get_short( dateArray.data() );
	struct tm t;
	t.tm_year = ((packed_date & 0xFE00) >> 9) + 4;
	t.tm_mon  = ((packed_date & 0x01E0) >> 5) - 1;
	t.tm_mday = (packed_date & 0x001F);
	t.tm_hour = 0;
	t.tm_min  = 0;
	t.tm_sec  = 0;
	t.tm_isdst= -1;
	
	data.lastChanged = readTm( t );
	
	return data;
}

void KeyringHHRecord::pack( const KeyringHHRecordBase &data )
{
	FUNCTIONSETUP;
	
	QByteArray unencryptedData;
	
	// Make sure that at least an empty string is added.
	if( data.account.isNull() )
	{
		unencryptedData.append( Pilot::toPilot( QString( "" ) ) );
	}
	else
	{
		unencryptedData.append( Pilot::toPilot( data.account ) );
	}
	unencryptedData.append( (char) 0x00 );
	
	if( data.password.isNull() )
	{
		unencryptedData.append( Pilot::toPilot( QString( "" ) ) );
	}
	else
	{
		unencryptedData.append( Pilot::toPilot( data.password ) );
	}
	unencryptedData.append( (char) 0x00 );
	
	if( data.notes.isNull() )
	{
		unencryptedData.append( Pilot::toPilot( QString( "" ) ) );
	}
	else
	{
		unencryptedData.append( Pilot::toPilot( data.notes ) );
	}
	unencryptedData.append( (char) 0x00 );
	
	// Copied from JPilot keyring conduit.
	tm dataLastChanged = writeTm( data.lastChanged );
	char lastChanged[2];
	
	unsigned short packedDate = ( ( ( dataLastChanged.tm_year - 4) << 9 ) & 0xFE00 )
		| ( ( ( dataLastChanged.tm_mon + 1 ) << 5 ) & 0x01E0 )
		| ( dataLastChanged.tm_mday & 0x001F );
	set_short( lastChanged, packedDate );
	// End of Copied code
	unencryptedData.append( lastChanged );
	
	// The encrypted portion must be a multiple of 8
	int size = unencryptedData.size();
	int zeroCount = 0;
	if( ( size % 8 ) )
	{
		zeroCount = ( 8 - ( size % 8 ) );
	}
	
	unencryptedData.append( QByteArray( zeroCount, (char) 0x00 ) );
	
	// Encrypt the stuff.
	QCA::Initializer init;
	QCA::SymmetricKey sKey( QCA::hexToArray( fKey ) );
	
	if( !QCA::isSupported( "tripledes-ecb" ) )
	{
		WARNINGKPILOT << "ERROR: tripledes not supported! Unable to continue."; 
		return;
	}

	QCA::Cipher cipher( "tripledes", QCA::Cipher::ECB
		, QCA::Cipher::NoPadding , QCA::Encode, sKey );
		
	QCA::SecureArray result = cipher.update( unencryptedData );
	cipher.final();
	
	QByteArray recordData;
	recordData.append( Pilot::toPilot( name() ) );
	recordData.append( (char) 0x00 );
	recordData.append( result.toByteArray() );
	
	pi_buffer_t *buf = pi_buffer_new( recordData.size() );
	buf->used = recordData.size();
	memcpy( buf->data, (unsigned char*) recordData.data(), recordData.size() );
	
	fRecord->setData( buf );
}

QString KeyringHHRecord::toString() const
{
	QString flags( '[' );

	fRecord->isModified() ? flags.append( 'M' ) : flags.append( '-' );
	fRecord->isArchived() ? flags.append( 'A' ) : flags.append( '-' );
	fRecord->isDeleted() ? flags.append( 'D' ) : flags.append( '-' );
	flags.append( ']' );

	return id() + CSL1( " - " ) + fName + CSL1( " " ) + flags;
}

void KeyringHHRecord::setModified()
{
	fRecord->setModified();
}
