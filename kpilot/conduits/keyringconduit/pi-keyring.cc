#include "pi-keyring.h"

#include <QtDebug>

#if PILOT_LINK_IS(0,12,2)
int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
	const unsigned char *record, size_t len )
#else
int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
	unsigned char *record, size_t len )
#endif
{
	return unpack_CategoryAppInfo( &ai->category, record, len + 4 );
}

#if PILOT_LINK_IS(0,12,2)
int pack_KeyringAppInfo( const KeyringAppInfo_t *ai, 
	unsigned char *record, size_t len )
#else
int pack_KeyringAppInfo( KeyringAppInfo_t *ai, 
	unsigned char *record, size_t len )
#endif
{
	int i = pack_CategoryAppInfo( &ai->category, record, len );
	qDebug() << "Length:" << i << len;
	return i;
}
