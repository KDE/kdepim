#include <iostream>

#include <qcstring.h>
#include <qstring.h>
#include <qlist.h>

#include "VCardEnum.h"
#include "VCardVCard.h"
#include "VCardVCardEntity.h"
#include "VCardEmailParam.h"
#include "VCardContentLine.h"
#include "VCardNValue.h"
#include "VCardTextValue.h"

#include "Entity.h"
#include "KAddressBook.h"

extern "C" {
int import(const char * data, const char * addressBookName);
}

int import(const char * data, KAB::AddressBook * ab) 
{
  Q_UINT32 imported(0);
  
  QCString str(data);
  
  VCARD::VCardEntity v(str);
  VCARD::VCardList cardList = v.cardList();
  VCARD::VCardListIterator it(cardList);
    
  for (int imported = 0; it.current(); ++it, imported++) {

    Entity e;
    
    VCARD::ContentLineList contentLineList = it.current()->contentLineList();

    VCARD::ContentLineListIterator line_it(contentLineList);

    for (; line_int.current(); ++line_it) {
      
      VCARD::ContentLine * line = line_it.current();
      
      // Ignore grouping for the moment.
      
      QCString  value = line->value()->asString();
      QCString  name  = line->name();

      QByteArray data = value.data();
      int dataLength = value.length();
      
    }
  }

  return imported;
}
