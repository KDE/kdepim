

#ifndef KSYNC_MAN_PART_SERVICE_H
#define KSYNC_MAN_PART_SERVICE_H

#include <qvaluelist.h>
#include <qstring.h>

#include <kservice.h>

namespace KSync {
   /**
    * a ManPartServive saves a converted
    * KService::Ptr of a ManipulatorPart
    * @see KService::Ptr
    */
    class ManPartService {
    public:
        /**
         * creates an Empty Service
         */
        typedef QValueList<ManPartService> ValueList;
        ManPartService();
        ManPartService( const KService::Ptr& service );
        ManPartService( const ManPartService& );
        ~ManPartService();
        bool operator==( const ManPartService& );
        bool operator==( const ManPartService& )const;
        QString name()const;
        QString comment()const;
        QString libname()const;
        QString icon() const;

        void setName(const QString & );
        void setComment( const QString& comment );
        void setLibname( const QString& );
        void setIcon( const QString& );

        ManPartService &operator=( const ManPartService&);
    private:
        QString m_name;
        QString m_comment;
        QString m_icon;
        QString m_lib;
    };
};


#endif
