#ifndef KSYNC_OPIE_EXTRA_TAGS_MAP_H
#define KSYNC_OPIE_EXTRA_TAGS_MAP_H

#include <qmap.h>
#include <qstring.h>


class QDomNamedNodeMap;
class QStringList;
namespace OpieHelper {

    /**
     * Whenever we do not handle a XML attribute we need
     * to save it somewhere and when we write we need to flush
     * the tags to file
     * This way we won't loose changes
     */

    /**
     *used to save on a Key Value basis
     */
    typedef QString CUID; // ComposedUID
    typedef QMap<QString, QString> KeyValue;
    typedef QMap<CUID, KeyValue> ExtraMapBase;

    struct ExtraMap : public ExtraMapBase {
         /**
          * Converts the KeyValue
          */
         QString toString( const CUID& );

         /**
          * assembles 'app-uid' and converts the stuff to additional attributes
          */
         QString toString( const QString& app, const QString& uid );

         /**
          * add a CUID with keyValue
          * @param app the Application
          * @param uid The uid
          * @param map The AttributeMap
          * @param lst The list of handled attributes
          */
         void add(const QString& app, const QString& uid, const QDomNamedNodeMap& map, const QStringList& lst);

    protected:
        QString escape( const QString& str );

     };

}


#endif
