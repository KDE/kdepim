
#ifndef KSYNC_PROFILE_MANAGER_H
#define KSYNC_PROFILE_MANAGER_H

#include <qstring.h>

#include <profile.h>

namespace KSync {

    /**
     * ProfileManager keeps track of the Profiles
     * It allows you to retrieve and set the current.
     * remove and add new/old Profiles
     * Load and Save from KConfig
     */
    class ProfileManager  {
    public:
        /**
         * Constructs an Empty ProfileManager
         */
        ProfileManager();

        /**
         * Constructs a profile manager from a Profile List.
         */
        ProfileManager( const Profile::ValueList& list );

        /**
         * Destructs a profile manager
         */
        ~ProfileManager();

        /**
         * returns the current active Profile
         */
        Profile currentProfile()const;

        /**
         * sets the current Profile
         */
        void setCurrentProfile( const Profile& profile );

        /**
         * returns a list of all active profiles
         */
        Profile::ValueList profiles()const;

        /**
         * set the Manager to use a list of Profiles
         */
        void setProfiles( const Profile::ValueList& list );

        /**
         * is finding a Profile by name
         */
        Profile byName( const QString& name );

        /**
         * returns a profile list of of Profiles matching name
         */
        Profile::ValueList byName2( const QString& name );

        /*
         * profile at index
         */
        Profile profile( int index )const;

        /**
         * the count of elements
         */
        int count()const;

        /**
         * loads a Profile List
         */
        void load();

        /**
         * saves current list including current Profile
         */
        void save();

        /**
         * add a Profile
         */
        void addProfile( const Profile& );

        /**
         * replaces a profile
         */
        void replaceProfile( const Profile& );

        /**
         * removes a Profile
         */
        void removeProfile( const Profile& );

    private:
        Profile m_cur;
        Profile::ValueList m_list;
    };
};


#endif
