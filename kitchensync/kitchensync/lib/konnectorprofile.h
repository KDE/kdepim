
#ifndef KSYNC_KONNECTOR_PROFILE_H
#define KSYNC_KONNECTOR_PROFILE_H

#include <qstring.h>
#include <qvaluelist.h>

#include <kapabilities.h>
#include <kdevice.h>


class KConfig;
namespace KSync {

    /**
     * A KonnectorProfile stores the name and a KSync::Device,
     * a Kapabilities and other configuration related
     * to a Konnector
     * @base a simple KonnectorProfile
     * @author Holger 'zecke' Freyther
     */
    class KonnectorProfile {
    public:
        typedef QValueList<KonnectorProfile> ValueList;

	/**
	 * a simple c'tor
	 * @base simple c'tor
	 */
        KonnectorProfile();

	/**
	 * Another constructor
	 * @param name The name of the Konnector assigned by the user
	 * @param icon A user supplied icon
	 * @param Device the device
	 */
        KonnectorProfile(const QString& name,
                         const QString& icon,
                         const Device& dev);

	/**
	 * A simple copy constructor
	 */
        KonnectorProfile( const KonnectorProfile& );
        ~KonnectorProfile();
        bool operator==( const KonnectorProfile& );
        bool operator==( const KonnectorProfile& )const;
        KonnectorProfile &operator=( const KonnectorProfile& );

	/**
	 * The UID gets assigned by the MainWindow
	 * @see MainWindow
	 * @return the UID ( not UDI ) of the Konnector
	 */
        QString uid()const;

	/**
	 * @return the user supplied name
	 */
        QString name()const;

	/**
	 * @return the icon name
	 */
        QString icon()const;

	/**
	 * @return the device
	 */
        Device device()const;

	/**
	 * @return thde udi of the Konnector. Only valid if the KonnectorProfile is loaded
	 */
        QString udi() const;

	/**
	 * @return the kapabilities  of this KonnectorProfile
	 * @see Kapabilities
	 */
        Kapabilities kapabilities() const;

	/**
	 * @return if the KonnectorProfile was
	 * 	   loaded in a previous session
	 */
        bool wasLoaded()const;

	/**
	 * set the uid of the KonnectorProfile
	 * @param uid The uid of the KonnectorProfile
	 */
        void setUid( const QString& uid);

	/**
	 * set the name of the KonnectorProfile
	 * @param name The name of the KonnectorProfile
	 */
        void setName( const QString& name );

	/**
	 * set the icon of the KonnectorProfile
	 * @param icon the icon name
	 */
        void setIcon( const QString& icon );

	/**
	 * set the Device of this KonnectorProfile
	 * @param dev The device
	 */
        void setDevice( const Device& dev );

	/**
	 * set the UDI of the KonnectorProfile
	 * @param udi The UDI of a KonnectorPlugin
	 */
        void setUdi( const QString& udi );

	/**
	 * set the Kapabilities for the KonnectorProfile
	 * @param caps The Kapabilities
	 */
        void setKapabilities( const Kapabilities& caps);

        /**
         * @return if this KonnectorProfile is valid
         * It is valid if it got a proper name!
         */
        bool isValid()const;

	/**
	 * @internal used for storing to a KConfig
	 */
        void saveToConfig( KConfig* )const;

	/**
	 * @internal loading from a Kconfig
	 */
        void loadFromConfig( KConfig* );

    private:
        void saveKaps( KConfig* )const;
        Kapabilities readKaps( KConfig* );
        bool m_wasLoaded :1;
        QString m_name;
        QString m_icon;
        Device m_dev;
        QString m_uid;
        QString m_udi;
        Kapabilities m_caps;
        class KonnectorProfilePrivate;
        KonnectorProfilePrivate *d;
    };
};

#endif
