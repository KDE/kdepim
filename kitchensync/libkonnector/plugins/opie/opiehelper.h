

#ifndef opiehelper_h
#define opiehelper_h

#include <qptrlist.h>

#include <ksyncentry.h>
#include <kalendarsyncentry.h>
#include <kaddressbooksyncentry.h>g
#include <opiedesktopsyncentry.h>
#include "opiecategories.h"

class OpieHelper {
 public:
    static void toOpieDesktopEntry( const QString &, QPtrList<KSyncEntry> *list, const QValueList<OpieCategories> & );
    static void toCalendar(const QString &fileName , QPtrList<KSyncEntry> *list, const QValueList<OpieCategories> &);
    static void toAddressbook( const QString &fileName, QPtrList<KSyncEntry> *list, const QValueList<OpieCategories> & );
};

#endif

