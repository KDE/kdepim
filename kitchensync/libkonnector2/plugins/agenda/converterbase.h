#ifndef VR3_CONVERTER_BASE_H
#define VR3_CONVERTER_BASE_H

#include <sys/types.h>
#include <time.h>

#include <qdatetime.h>
#include <qstring.h>

#include <kontainer.h>
#include <syncer.h>

#include <idhelper.h> // the Opie and PAlm uids are not random enough the id helper helps to map them to strong UIDs on the KDE side

class KTempFile;
namespace Vr3 {
    /**
     * A base class for all converters
     * This class will contain convience functions
     * which are needed by all AgendaVr3 converters
     */
    class ConverterBase {
    public:
        /**
         * a pointer to the UID helper
         * and the name of the timezone!
         */
        ConverterBase( KSync::KonnectorUIDHelper* helper,
                       const QString& timeZone );
        ~ConverterBase();

    protected:
        time_t toUTC( const QDateTime& time );
        QDateTime fromUTC( time_t );

        /** creates a new KTempFile */
        KTempFile* file();

        /** return the timezone */
        QString timeZone()const;

        /** re returns the Konnector-Side id for a KDE UID */
        QString konnectorId( const QString& appName, const QString& uid );

        /** returns a KDE UID for a Konnector Id */
        QString kdeId( const QString& appName, const QString& uid );

    private:
        KSync::KonnectorUIDHelper* m_helper;
        QString m_tz;


    };
}


#endif
