#ifndef KRSS_DBUSHELPER_H
#define KRSS_DBUSHELPER_H

#include <climits>

class QDBusAbstractInterface;
class QObject;
class QString;

template <typename T> class QList;
class QVariant;

namespace KRss {

    namespace DBusHelper {
        enum Timeout {
            DefaultTimeout=-1,
            NoTimeout=INT_MAX
        };

        bool callWithCallback( QDBusAbstractInterface* iface, const QString& method, const QList<QVariant>& args, QObject* receiver, const char* slot, const char* errorMethod, Timeout timeout=DefaultTimeout );
    }
}

#endif // KRSS_DBUSHELPER_H
