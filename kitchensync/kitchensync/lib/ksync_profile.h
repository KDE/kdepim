#ifndef KitchenSync_Profile_H
#define KitchenSync_Profile_H

#include <qmap.h>
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
        typedef QMap<QString,  QString> PathMap;
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
         * @return the user given name of the profile
         */
        QString name()const;

        /**
         * @return the uid of the Profile
         */
        QString uid() const;

        /**
         * @return a name of a Pixmap the use
         * chose to associate
         */
        QString pixmap() const;

        /**
         * set the name
	 * @param name the name of the Profile
         */
        void setName( const QString& name ) ;

        /**
         * set the uid
	 * @param id the id of the Profile
         */
        void setUid( const QString& id );

        /**
         * set the Pixmap name
	 * @param pix The pixmap
         */
        void setPixmap( const QString& pix);
	
        /**
         * @return the ManipulatorParts to be loaded for
         * the profile
         */
        ManPartService::ValueList manParts()const;

        /**
         * set which parts to be loaded
	 * @param lst The list of ManPartServices
         */
        void setManParts( const ManPartService::ValueList& lst);

        /**
         * Parts can save the file location inside a Profile
         * path returns the PATH for a part
         */
        QString path( const QString& partName )const;

        /**
         * sets the path for a partName
         * to path
	 * @partName The part name
	 * @path the path
         */
        void setPath( const QString& partName,  const QString& path );

        /**
         * sets the PATH MAp
         */
        void setPaths( const PathMap& );

        /**
         * returns the PathMap
         */
        PathMap paths() const;

        /**
         * copy operator;
         */
        Profile &operator=(const Profile & );

    private:
        QString m_name;
        QString m_uid;
        QString m_pixmap;
        ManPartService::ValueList m_list;
        PathMap m_map;
    };

};


#endif
