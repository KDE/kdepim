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
	
	if( f == eAccount )
	{
		return QString( result.toByteArray() );
	}
	
	return QVariant();
}

QString KeyringHHRecord::toString() const
{
	FUNCTIONSETUP;
	
	return id() + " - " + fName;
}
