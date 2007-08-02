#include "keyringhhrecord.h"

#include "options.h"
#include "pilotRecord.h"

KeyringHHRecord::KeyringHHRecord( PilotRecord *rec, const QCA::SymmetricKey &key)
	: HHRecord( rec ), fKey( key )
{
	qDebug() << "Key:" << QString::fromUtf8( fKey.toByteArray() );
	QString name( fRecord->data() );
	int n = name.size() + 1; // Stringlengt + zero.
	int size = fRecord->size();

	QCA::Initializer init;
	QCA::SecureArray encryptedData(
		QByteArray( fRecord->data(), size ).right( size - n) );
	
	// Encrypted data is in fRecord->data()[n..size]
	QCA::Cipher::Cipher cipher( "tripledes", QCA::Cipher::ECB
		, QCA::Cipher::NoPadding , QCA::Decode, fKey );
	QCA::SecureArray result = cipher.update( encryptedData );
	cipher.final();
	
	QString result1 = result.toByteArray();
	qDebug() << "TEST: " << result.size() << result1;
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

QString KeyringHHRecord::toString() const
{
	FUNCTIONSETUP;
	
	// QString reads until the first 0.
	QString name( fRecord->data() );
	
	// For some reason fKey has no data here anymore.
	qDebug() << "Key:" << QString::fromUtf8( fKey.toByteArray() );
	
	return name; //+ " - " + result1;
}
