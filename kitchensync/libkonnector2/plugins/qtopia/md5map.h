#ifndef MD5_MAP_H
#define MD5_MAP_H

#include <qcstring.h>
#include <qmap.h>

/**
 * MD5 Map is here to keep
 * a Map of UID and a MD5 SUM
 * together
 * It can save/load, find and
 * iterate over a list of items
 */
class KConfig;

namespace OpieHelper {
    class MD5Map {
    public:
        typedef QMap<QString, QString> Map;
        typedef QMap<QString, QString>::Iterator Iterator;
        MD5Map(const QString& fileName = QString::null );
        ~MD5Map();
        void load( const QString& fileName );

        /* clears before saving */
        void save();
        /* only works if not loaded before */
        void setFileName( const QString& );

        QString md5sum(const QString& )const;
        bool contains( const QString& )const;
        void insert( const QString& , const QString& );
        void set( const Map& map );

        Map map()const;

        void clear();

    protected:
        KConfig* config();

    private:
        KConfig* m_conf;
        Map m_map;
        QString m_file;
    };
};

#endif
