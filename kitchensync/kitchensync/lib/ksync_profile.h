#ifndef KitchenSync_Profile_H
#define KitchenSync_Profile_H

#include <qstring.h>

#include <kapabilities.h>
#include <kdevice.h>


namespace KSync {

    class Profile {
    public:
        Profile();
        Profile( const Device&,  const Kapabilities&,  const QString& name, bool enable );
        Profile(const Profile& prof );
        ~Profile();
        Device device()const;
        QString name()const;
        bool isConfigured()const;
        Kapabilities caps()const;

        void setDevice( const Device& dev) {m_device = dev; };
        void setName( const QString& name ) { m_name = name; };
        void setConfigured( bool conf ) { m_configured = conf; };
        void setCapability( const Kapabilities& cap ) { m_caps = cap; };
        Profile &operator=(const Profile & );
    private:
        QString m_name;
        Device m_device;
        bool m_configured:1;
        Kapabilities m_caps;
    };
};


#endif
