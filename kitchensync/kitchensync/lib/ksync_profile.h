#ifndef KitchenSync_Profile_H
#define KitchenSync_Profile_H

#include <qstring.h>

#include <kapabilities.h>
#include <kdevice.h>

#include <manpartservice.h>

namespace KSync {

    /**
     * A Profile keeps user settings like a name,
     * the list of plugins to be loaded on activation,
     * a list of where to read data from....
     */
    class Profile {
    public:
        /** some kewl operators */
        bool operator==( const Profile& );
//        bool operator!=( const Profile& a) { return !(a == *this); };
        typedef QValueList<Profile> ValueList;
        /**
         * constructs an empty Profile
         * and generates a uid
         */
        Profile();

        /**
         * copy c'tor
         */
        Profile( const Profile& );

        /**
         * destructs a Profile
         */
        ~Profile();

        /**
         * returns the user given name of the profile
         */
        QString name()const;

        /**
         * returns the uid of the Profile
         */
        QString uid() const;

        /**
         * returns a name of a Pixmap the use
         * chose to associate
         */
        QString pixmap() const;

        /**
         * set the name
         */
        void setName( const QString& name ) ;

        /**
         * set the uid
         */
        void setUid( const QString& id );

        /**
         * set the Pixmap name
         */
        void setPixmap( const QString& );
        /**
         * returns the ManipulatorParts to be loaded for
         * the profile
         */
        ManPartService::ValueList manParts()const;

        /**
         * set which parts to be loaded
         */
        void setManParts( const ManPartService::ValueList& );

        /**
         * copy operator;
         */
        Profile &operator=(const Profile & );

    private:
        QString m_name;
        QString m_uid;
        QString m_pixmap;
        ManPartService::ValueList m_list;
    };

};


#endif
