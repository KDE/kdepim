
#ifndef KSYNC_KONNECTOR_PROFILE_H
#define KSYNC_KONNECTOR_PROFILE_H

#include <qstring.h>
#include <qvaluelist.h>

#include <kapabilities.h>
#include <kdevice.h>


class KConfig;
namespace KSync {

    /**
     * A KonnectorProfile stores the name and a KSync::Device
     * and maybe a pixmap location
     */
    class KonnectorProfile {
    public:
        typedef QValueList<KonnectorProfile> ValueList;
        KonnectorProfile();
        KonnectorProfile(const QString& name,
                         const QString& icon,
                         const Device& dev);
        KonnectorProfile( const KonnectorProfile& );
        ~KonnectorProfile();
        bool operator==( const KonnectorProfile& );
        bool operator==( const KonnectorProfile& )const;
        KonnectorProfile &operator=( const KonnectorProfile& );

        QString uid()const;
        QString name()const;
        QString icon()const;
        Device device()const;
        QString udi() const;
        Kapabilities kapabilities() const;
        bool wasLoaded()const;

        void setUid( const QString& );
        void setName( const QString& name );
        void setIcon( const QString& icon );
        void setDevice( const Device& dev );
        void setUdi( const QString& udi );
        void setKapabilities( const Kapabilities& );

        void saveToConfig( KConfig* )const;
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
