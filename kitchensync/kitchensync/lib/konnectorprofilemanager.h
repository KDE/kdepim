
#ifndef KSYNC_KONNECTOR_PROFILE_MANAGER
#define KSYNC_KONNECTOR_PROFILE_MANAGER

#include <konnectorprofile.h>

namespace KSync {
    /**
     * Manages loading and saving of profiles
     * Manages current Profiles... adding, removing
     * and finding them
     */
    class KonnectorProfileManager {
    public:
        KonnectorProfileManager(const KonnectorProfile::ValueList& );
        KonnectorProfileManager();
        ~KonnectorProfileManager();

        /**
         * find searches for @param id inside the uid and the name
         * of profiles
         */
        KonnectorProfile find( const QString& id );
        KonnectorProfile find( const Device& dev );

        KonnectorProfile::ValueList list() const;
        void setList( const KonnectorProfile::ValueList& list );

        KonnectorProfile current() const;
        void setCurrent( const KonnectorProfile& );

        void add( const KonnectorProfile& );
        void replace( const KonnectorProfile& prof);
        void remove( const KonnectorProfile& );
        void clear();
        int count()const;
        KonnectorProfile profile(int index )const;

        void load();
        void save();

    private:
        KonnectorProfile::ValueList m_list;
        KonnectorProfile m_current;

    };
};

#endif
