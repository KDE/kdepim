#include "keyringhhrecord.h"

KeyringHHRecord::KeyringHHRecord( PilotRecord *rec ) : HHRecord( rec )
{
}

bool KeyringHHRecord::equal( const Record* other ) const
{
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
