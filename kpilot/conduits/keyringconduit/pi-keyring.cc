#include "pi-keyring.h"

#include <QtDebug>

int unpack_KeyringAppInfo( KeyringAppInfo_t *ai
	, unsigned char *record, size_t len )
{
	return unpack_CategoryAppInfo( &ai->category, record, len + 4 );
}

int pack_KeyringAppInfo( KeyringAppInfo_t *ai, unsigned char *record
	, size_t len )
{
	int i = pack_CategoryAppInfo( &ai->category, record, len );
	qDebug() << "Length:" << i << len;
	return i;
}
