#include <qcstring.h>
#include <qstring.h>
#include <qlist.h>

#include "VCardEnum.h"
#include "VCardNValue.h"
#include "VCardTextValue.h"
#include "VCardVCardEntity.h"

#include "KabAddressBook.h"
#include "KabEnum.h"
#include "KabPerson.h"
#include "KabPersonalName.h"

extern "C" {
int import(const char *, KAB::AddressBook *);
}

int import(const char * _str, KAB::AddressBook * ab) 
{
  Q_UINT32 imported(0);
  
  QCString str(_str);
  
  using namespace VCARD;
  
  VCardEntity v(str);
  VCardList cardList = v.cardList();
  VCardListIterator it(cardList);
  
  for (; it.current(); ++it) {
    
    VCard & v = *it.current();
    
    if (v.has(EntityN) && v.has(EntityFullName)) {
      
      NValue * name =
        (NValue *)(v.contentLine(EntityN)->value());
      
      TextValue * fullName =
        (TextValue *)(v.contentLine(EntityFullName)->value());
      
      using namespace KAB;
      
      Person * p = new Person(*ab, QString(fullName->asString()));
      
      PersonalName pn;
      pn.setPrefixes    (QString(name->prefix()));
      pn.setFirstName   (QString(name->given()));
      pn.setOtherNames  (QString(name->middle()));
      pn.setLastName    (QString(name->family()));
      pn.setSuffixes    (QString(name->suffix()));
      
      p->setPersonalName(pn);
  
      ++imported;
    }
  }

  return imported;
}
