#include <qstringlist.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qshared.h>
#include <qdict.h>
#include <qlist.h>
#include <qregexp.h>

#ifndef KAB_ENUM_H
#define KAB_ENUM_H

namespace KAB
{

enum    Gender        { GenderMale,   GenderFemale, GenderOther };
enum    LocationType  { LocationWork, LocationHome, LocationOther };

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

typedef QArray<char> CharBuf;
typedef CharBuf Data;

} // End namespace KAB

#endif

