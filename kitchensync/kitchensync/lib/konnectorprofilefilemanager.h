
#ifndef KSYNC_KONNECTORPROFILEFILEMANAGER_H
#define KSYNC_KONNECTORPROFILEFILEMANAGER_H

#include "konnectorprofile.h"

namespace KSync {

/**
 *  The KonnectorProfileManager loads
 *  and saves a set of KonnectorProfiles
 *  to and from the disk
 *  @short saves a list of KonnectorProfiles
 */
class KonnectorProfileFileManager
{
  public:
    /**
     * simple c'tor
     */
    KonnectorProfileFileManager();
    ~KonnectorProfileFileManager();

    /**
     * Load the list of KonnectorProfiles from the disk
     * @return the list of KonnectorProfiles
     */
    KonnectorProfile::ValueList load();

    /**
     * Save the list of profiles
     * @param list The list of profiles
     */
    void save( const KonnectorProfile::ValueList &list );
};

}

#endif
