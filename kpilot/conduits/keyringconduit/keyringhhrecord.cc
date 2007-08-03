#include "keyringhhrecord.h"

#include "options.h"
#include "pilotRecord.h"

KeyringHHRecord::KeyringHHRecord( PilotRecord *rec, const QString &key)
	: HHRecord( rec ), fKey( key )
{
	fName = QString( fRecord->data() );
}

bool KeyringHHRecord::equal( const Record* other ) const
{
	FUNCTIONSETUP;
	
	const KeyringHHRecord *rec = dynamic_cast<const KeyringHHRecord*>( other );
	
	if( rec )
	{
		return false;
	}
	else
	{
		return false;
	}
}

QString KeyringHHRecord::name() const
{
	FUNCTIONSETUP;
	
	return fName;
}

QString KeyringHHRecord::account() const
{
	FUNCTIONSETUP;
	
	return getEncryptedField( eAccount ).toString();
}

QString KeyringHHRecord::password() const
{
	FUNCTIONSETUP;
	
	return getEncryptedField( ePassword ).toString();
}

QString KeyringHHRecord::notes() const
{
	FUNCTIONSETUP;
	
	return getEncryptedField( eNotes ).toString();
}

QDateTime KeyringHHRecord::lastChangedDate() const
{
	FUNCTIONSETUP;
	
	return getEncryptedField( eLastChangeTime ).toDateTime();
}

QVariant KeyringHHRecord::getEncryptedField( const Field f ) const
{
	int n = fName.size() + 1; // Stringlengt + zero.
	int size = fRecord->size();

	QCA::Initializer init;
	QCA::SecureArray encryptedData(
		QByteArray( fRecord->data(), size ).right( size - n) );
	
	// Encrypted data is in fRecord->data()[n..size]
	QCA::SymmetricKey sKey( QCA::hexToArray( fKey ) );
	QCA::Cipher::Cipher cipher( "tripledes", QCA::Cipher::ECB
		, QCA::Cipher::NoPadding , QCA::Decode, sKey );
	QCA::SecureArray result = cipher.update( encryptedData );
	cipher.final();
	
	int pos = 0;
	
	QString account;
	QString pass;
	QString notes;
	QByteArray date;
	
	for( int i = 0; i < result.size(); i++ )
	{
		if( result[i] )
		{
			switch( pos )
			{
				case 0:
					account.append( QChar( result[i] ) );
					break;
				case 1:
					pass.append( QChar( result[i] ) );
					break;
				case 2:
					notes.append( QChar( result[i] ) );
					break;
				case 3:
					date.append( result[i] );
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
	
	switch( f )
	{
		case eAccount:
			return account;
		case ePassword:
			return pass;
		case eNotes:
			return notes;
		case eLastChangeTime:
			struct tm t;
			
			// Copied from keyring.c (j-pilot conduit)
			unsigned short packed_date = get_short( date.data() );
			t.tm_year = ((packed_date & 0xFE00) >> 9) + 4;
			t.tm_mon  = ((packed_date & 0x01E0) >> 5) - 1;
			t.tm_mday = (packed_date & 0x001F);
			t.tm_hour = 0;
			t.tm_min  = 0;
			t.tm_sec  = 0;
			t.tm_isdst= -1;
			
			QDateTime dateTime = readTm( t );
			return dateTime;
	}
	
	return QVariant();
}

QString KeyringHHRecord::toString() const
{
	FUNCTIONSETUP;
	
	return id() + " - " + fName;
}
