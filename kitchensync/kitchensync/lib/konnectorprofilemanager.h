
#ifndef KSYNC_KONNECTOR_PROFILE_MANAGER
#define KSYNC_KONNECTOR_PROFILE_MANAGER

#include "konnectorprofile.h"

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
	 * @return a KonnectorProfile
         */
        KonnectorProfile find( const QString& id );

	/**
	 * @return a KonnectorProfile for Device
	 */
        KonnectorProfile find( const Device& dev );

	/**
	 * @return the list of KonnectorProfiles
	 */
        KonnectorProfile::ValueList list() const;

	/**
	 * set the internal ProfileList to @param list
	 * @param list the list of KonnectorProfiles
	 */
        void setList( const KonnectorProfile::ValueList& list );

	/**
	 * @return the currently activated KonnectorProfile
	 */
        KonnectorProfile current() const;

	/**
	 * set the current KonnectorProfile
	 * @param prof The profile
	 */
        void setCurrent( const KonnectorProfile& prof);

	/**
	 * add a KonnectorProfile
	 * @param prof The KonnectorProfile
	 */
        void add( const KonnectorProfile& prof);

	/**
	 * replace the KonnectorProfile
	 * @param prof the KonnectorProfile
	 */
        void replace( const KonnectorProfile& prof);

	/**
	 * remove the KonnectorProfile from the internal list
	 * @param prof The KonnectorProfile
	 */
        void remove( const KonnectorProfile& prof);

	/**
	 * clear the list of profiles
	 */
        void clear();

	/**
	 * @return the number of KonnectorProfiles
	 */
        uint count()const;

	/**
	 * @return the KonnectorProfile at index
	 * @param index the index of the KonnectorProfile
	 */
        KonnectorProfile profile(uint index )const;

	/**
	 * load
	 */
        void load();

	/**
	 * save
	 */
        void save();

    private:
        KonnectorProfile::ValueList m_list;
        KonnectorProfile m_current;

    };
};

#endif
