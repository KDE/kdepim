#include <qdom.h>
#include <qstylesheet.h>
#include <qstringlist.h>

#include <kdebug.h>

#include "extramap.h"

using namespace OpieHelper;


QString ExtraMap::toString( const CUID& cuid) {
    if (!contains( cuid ) ) return QString::null;

    KeyValue val = (*this)[cuid];
    KeyValue::Iterator it;
    QString str;
    for (it = val.begin(); it != val.end(); ++it ) {
        str += " "+it.key()+"=\""+escape( it.data() )+"\"";
    }

    return str;
}
QString ExtraMap::toString( const QString& app, const QString& uid ) {
    return toString(app+uid);
}
void ExtraMap::add( const QString& app, const QString& uid, const QDomNamedNodeMap& map, const QStringList& lst ) {
    KeyValue val;
    uint count =  map.count();
    for ( uint i = 0; i < count; i++ ) {
        QDomAttr attr = map.item( i ).toAttr();
        if (!attr.isNull() ) {
            if (!lst.contains(attr.name() ) ) {
                val.insert( attr.name(), attr.value() );
            }
        }
    }
    insert(app+uid, val );
}
QString ExtraMap::escape( const QString& str ) {
    return QStyleSheet::escape( str );
}
