#ifndef KAB_ENUM_H
#define KAB_ENUM_H

#include <qstring.h>
#include <qstrlist.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <kurl.h>


namespace KAB
{

class AddressBook;
  
enum    EntityType {
  EntityTypeEntity,
  EntityTypeGroup
};

typedef QString UniqueID;

typedef UniqueID GroupRef;
typedef UniqueID EntityRef;

typedef QValueList<GroupRef>  GroupRefList;
typedef QValueList<EntityRef> EntityRefList;

typedef   void * FilterHandle;
typedef   void * KabBackendHandle;
 
static int       (*filterfn)             (const char *, AddressBook *);
static void      (*kab_backend_init)     (const KURL &);
static bool      (*kab_backend_read)     (const QCString &, QByteArray &);
static bool      (*kab_backend_write)    (const QCString &, const QByteArray &);
static bool      (*kab_backend_remove)   (const QCString &);
static void      (*kab_backend_all_keys) (QStrList &);

typedef   typeof(filterfn)              Filter;
typedef   typeof(kab_backend_init)      backendInit;
typedef   typeof(kab_backend_read)      backendRead;
typedef   typeof(kab_backend_write)     backendWrite;
typedef   typeof(kab_backend_remove)    backendRemove;
typedef   typeof(kab_backend_all_keys)  backendAllKeys;


} // End namespace KAB

#endif

