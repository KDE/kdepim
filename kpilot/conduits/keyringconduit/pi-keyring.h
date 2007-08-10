#ifndef PIKEYRING_H
#define PIKEYRING_H

#include "pilotAppInfo.h"

typedef struct KeyringAppInfo {
	struct CategoryAppInfo category;
} KeyringAppInfo_t;

#if PILOT_LINK_IS(0,12,2)
	int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		const unsigned char *record, size_t len );
	int pack_KeyringAppInfo( const KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
#else  
	int unpack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
	int pack_KeyringAppInfo( KeyringAppInfo_t *ai, 
		unsigned char *record, size_t len );
#endif


typedef PilotAppInfo<struct KeyringAppInfo, unpack_KeyringAppInfo
	, pack_KeyringAppInfo> PilotKeyringInfo;

#endif
