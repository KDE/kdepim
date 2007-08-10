#ifndef PIKEYRING_H
#define PIKEYRING_H

#include "pilotAppInfo.h"

typedef struct KeyringAppInfo {
	struct CategoryAppInfo category;
} KeyringAppInfo_t;

int unpack_KeyringAppInfo( KeyringAppInfo_t *ai
	, unsigned char *record, size_t len );
	
int pack_KeyringAppInfo( KeyringAppInfo_t *ai, unsigned char *record
	, size_t len );

typedef PilotAppInfo<struct KeyringAppInfo, unpack_KeyringAppInfo
	, pack_KeyringAppInfo> PilotKeyringInfo;

#endif
