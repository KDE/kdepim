

#ifndef opiehelper_h
#define opiehelper_h

#include <qptrlist.h>

#include <ksyncentry.h>
#include <kalendarsyncentry.h>
#include <kaddressbooksyncentry.h>
#include <opiedesktopsyncentry.h>
#include "opiecategories.h"

class OpieHelper {
 public:
  OpieHelper() {};
  ~OpieHelper();
  void toOpieDesktopEntry(  const QString &,
			   QPtrList<KSyncEntry> *list,
			   const QValueList<OpieCategories> & );

  void toCalendar(const QString &timeStamp,
		  const QString &todo, 
		  const QString &calendar ,
		  QPtrList<KSyncEntry> *list,
		  const QValueList<OpieCategories> &);

  void toAddressbook( const QString &timeStamp,
		      const QString &fileName,
		      QPtrList<KSyncEntry> *list,
		      const QValueList<OpieCategories> & );
  static OpieHelper *self();
 private:
  static OpieHelper *s_Self;
};

#endif

