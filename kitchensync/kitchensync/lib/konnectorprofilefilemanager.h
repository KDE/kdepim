
#ifndef KSYNC_KONNECTOR_PROFILE_FILE_MANAGER_H
#define KSYNC_KONNECTOR_PROFILE_FILE_MANAGER_H

#include <konnectorprofile.h>

namespace KSync{
    /**
     * @internal
     */
    class KonnectorProfileFileManager {
    public:
        KonnectorProfileFileManager();
        ~KonnectorProfileFileManager();

        KonnectorProfile::ValueList load();
        void save(const KonnectorProfile::ValueList&);

    };
};

#endif
