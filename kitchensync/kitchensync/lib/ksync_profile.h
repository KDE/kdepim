#ifndef KitchenSync_Profile_H
#define KitchenSync_Profile_H

#include <qstring.h>

#include <kapabilities.h>
#include <kdevice.h>


namespace KitchenSync {

    class Profile {
    public:
        Profile( const KDevice&,  const Kapabilities&,  const QString&, bool enable );
        Profile(const Profile& prof );
        ~Profile();
        KDevice device()const;
        QString name()const;
        bool isConfigured()const;
        Kapabilities caps()const;
        Profile &operator=(const Profile & );
    private:
        QString m_name;
        KDevice m_device;
        bool m_configured:1;
        Kapabilities m_caps;
    };
};


#endif
