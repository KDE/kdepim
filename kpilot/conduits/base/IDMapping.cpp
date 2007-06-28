
#include "IDMapping.h"

IDMapping::IDMapping() {
}

/**
 * Validates the mapping file with given dataproxy. The mapping is considered valid if:
 * 1. The number of mappings matches the number of records in the dataproxy.
 * 2. Every record that is in the backup database has a mapping.
 */
bool IDMapping::isValid() {
}

/**
 * Returns the pc record ID for given handheld record. Returns QString::Null if no mapping is found.
 */
QString IDMapping::pcRecordId() {
}

/**
 * Returns the id for the HH record which is mapped to the given pc record or 0 if there is no mapping.
 */
recordid_t IDMapping::hhRecordId() {
}

void IDMapping::setLastSyncedDate() {
}

void IDMapping::setLastSyncedPC() {
}

void IDMapping::save() {
}

 IDMapping::setPCId() {
}

 IDMapping::setHHId() {
}

/**
 * Creates a mapping for given records with id's.
 */
void IDMapping::map() {
}

bool IDMapping::contains() {
}

