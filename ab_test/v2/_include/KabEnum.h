#include <qstring.h>
#include <qstrlist.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <kurl.h>

#ifndef KAB_ENUM_H
#define KAB_ENUM_H


namespace KAB
{

class AddressBook;
  
enum    Gender        { GenderMale,   GenderFemale, GenderOther };
enum    LocationType  { LocationWork, LocationHome, LocationOther };
enum    EntityType {
  EntityTypeEntity,
  EntityTypeGroup,
  EntityTypePerson,
  EntityTypeLocation
};

typedef QString UniqueID;
typedef QString Division;

typedef UniqueID PersonRef;
typedef UniqueID LocationRef;
typedef UniqueID GroupRef;
typedef UniqueID MemberRef;

typedef QValueList<PersonRef>     PersonRefList;
typedef QValueList<LocationRef>   LocationRefList;
typedef QValueList<GroupRef>      GroupRefList;
typedef QValueList<MemberRef>     MemberRefList;

typedef QValueList<Division>      DivisionList;

int (*filterfn) (const char *, KAB::AddressBook *);

typedef   void *            FilterHandle;
typedef   typeof(filterfn)  Filter;

} // End namespace KAB

void      (*kab_backend_init)     (const KURL &);
bool      (*kab_backend_read)     (const QCString &, QByteArray &);
bool      (*kab_backend_write)    (const QCString &, const QByteArray &);
bool      (*kab_backend_remove)   (const QCString &);
void      (*kab_backend_all_keys) (QStrList &);

typedef   void * KabBackendHandle;

typedef   typeof(kab_backend_init)      backendInit;
typedef   typeof(kab_backend_read)      backendRead;
typedef   typeof(kab_backend_write)     backendWrite;
typedef   typeof(kab_backend_remove)    backendRemove;
typedef   typeof(kab_backend_all_keys)  backendAllKeys;



#endif

