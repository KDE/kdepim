#ifndef KSYNC_KONNECTOR_INFO_H
#define KSYNC_KONNECTOR_INFO_H

#include <qiconset.h>
#include <qstring.h>

namespace KSync {
    /**
     * Some informations about a konnector....
     */
    class KonnectorInfo {
    public:
        KonnectorInfo( const QString& name = QString::null,
                       const QIconSet& = QIconSet(),
                       const QString& id       = QString::null,
                       const QString& metaId   = QString::null,
                       const QString& iconName = QString::null,
                       const QString& udi      = QString::null,
                       bool isCon = false);

        ~KonnectorInfo();

        bool operator==( const KonnectorInfo& );

        QIconSet iconSet()const;
        QString id()const;
        QString metaId()const;
        QString iconName()const;
        QString udi()const;
        bool isConnected()const;
    private:
        QIconSet m_icon;
        QString m_id;
        QString m_meta;
        QString m_name;
        QString m_udi;
        bool m_con : 1;
        struct Data;
        Data *data;
        class Private;
        Private* d;
    };
}


#endif
