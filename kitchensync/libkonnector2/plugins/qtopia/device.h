#ifndef KSYNC_DEVICE_H
#define KSYNC_DEVICE_H

#include <qbitarray.h>
#include <qstring.h>
#include <qmap.h>

#include <kstaticdeleter.h>

namespace OpieHelper {
    class Device {
    public:
        enum Distribution {
            Opie, Zaurus
        };
        enum PIM {
            Calendar,
            Addressbook,
            Todolist
        };
        Device();
        ~Device();
        int distribution()const;
        void setDistribution(int dis );

        QBitArray supports( enum PIM );
        QString meta()const;
        void setMeta(const QString& str );

        QString user()const;
        void setUser( const QString& );
        QString password()const;
        void setPassword( const QString& );

    private:
        QBitArray opieCal();
        QBitArray opieTo();
        QBitArray opieAddr();
        int m_model;
        QString m_meta;
        QString m_user;
        QString m_pass;
    };
}


#endif
