/*
 * Implementation of KornStringId
 * Author: Mart Kelder
 */

#include "stringid.h"

KornStringId::KornStringId( const QString & id ) : _id( id )
{
}

KornStringId::KornStringId( const KornStringId & src ) : KornMailId(), _id( src._id )
{
}

KornMailId * KornStringId::clone() const
{
	return ( new KornStringId( *this ) );
}
					 
