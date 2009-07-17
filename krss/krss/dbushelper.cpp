#include "dbushelper_p.h"

#include <QtDBus/QDBusAbstractInterface>

#include <cassert>

using namespace KRss;
using namespace KRss::DBusHelper;

bool DBusHelper::callWithCallback( QDBusAbstractInterface* iface, const QString& method, const QList<QVariant>& args, QObject* receiver, const char* slot, Timeout timeout ) {

    QDBusMessage msg = QDBusMessage::createMethodCall( iface->service(), iface->path(), iface->interface(), method );
    msg.setArguments( args );
    return iface->connection().callWithCallback( msg, receiver, slot, static_cast<int>( timeout ) );
}
