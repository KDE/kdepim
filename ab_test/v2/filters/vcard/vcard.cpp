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

#include <KabEnum.h>
#include <KabAddressBook.h>
#include <KabPerson.h>
#include <KabPersonalName.h>
#include <KabGroup.h>

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
    
    KAB::Person * p = new KAB::Person(*ab, "As yet undefined");
      
    KAB::PersonalName pn;
    
    if (v.has(EntityN)) {
      
      NValue * name =
        (NValue *)(v.contentLine(EntityN)->value());
      
      pn.setPrefixes    (QString(name->prefix()));
      pn.setFirstName   (QString(name->given()));
      pn.setOtherNames  (QString(name->middle()));
      pn.setLastName    (QString(name->family()));
      pn.setSuffixes    (QString(name->suffix()));
    }
    
    if (v.has(EntityNickname)) {
      
      TextValue * nick =
        (TextValue *)(v.contentLine(EntityNickname)->value());
      
      pn.setNickName(QString(nick->asString()));
    }
      
    p->setPersonalName(pn);
  
    if (v.has(EntityFullName)) {
      
      TextValue * fullName =
        (TextValue *)(v.contentLine(EntityFullName)->value());
      
      p->setName(QString(fullName->asString()));
  
    }
    
    // Argh. Complexity.
    // Here, we get all the email addresses we can find and then try
    // to find which one is marked as preferred, which is then the only
    // one we keep. FIXME: We should really keep them all but we don't know
    // what they're for.

    int pref = -1;
    KAB::Comms comms;
    QValueList<KAB::EmailAddress> emailAddresses;
    
    QList<ContentLine> l = v.contentLineList();
    
    QListIterator<ContentLine> lit(l);  
    
    for (; lit.current(); ++lit) {
      
      if (lit.current()->entityType() == EntityEmail) {

        TextValue * email =
          (TextValue *)(lit.current()->value());
        
        QString e(email->asString());
        
        int i = e.find('@');
        
        KAB::EmailAddress a;
        a.setName(e.left(i));
        a.setDomain(e.mid(i) + 1);
        emailAddresses.append(a);
        ParamList params = lit.current()->paramList();
        
        QListIterator<Param> paramIt(params);
        for (; paramIt.current(); ++paramIt)
          if (((EmailParam *)paramIt.current())->pref())
            pref = emailAddresses.count() - 1;
      }  
    }
    
    if (emailAddresses.count() == 1)
      comms.setEmail(emailAddresses[0]);
    else if (emailAddresses.count() > 1)
      if (pref != -1)
        comms.setEmail(emailAddresses[pref]);
      else
        comms.setEmail(emailAddresses[0]);

    p->setContactInfo(comms);
   
    if (v.has(EntityOrganisation)) {
      
      // Well, if there's an organisation, then we shall see if a group
      // with the same name exists. If it does, we'll add to it, otherwise
      // we'll make a new one and put it in there.
      
      TextValue * org =
        (TextValue *)(v.contentLine(EntityOrganisation)->value());

      QString orgStr(org->asString());
      
      QStrList groups = ab->keysOfType(KAB::EntityTypeGroup);
      
      QStrListIterator it(groups);
      
      bool found(false);
      
      for (; it.current(); ++it) {
        KAB::Entity * e = ab->entity(it.current());
        if (e == 0) continue;
        if (e->name() == orgStr) {
          ((KAB::Group *)e)->addMember(*p);
          found = true;
          break;
        }
      }
      
      if (!found) {
        KAB::Group * g = new KAB::Group(*ab, orgStr);
        ((KAB::Group *)g)->addMember(*p);
        ab->update(g);
      }
      
    }
    
    ab->update(p);
    
    ++imported;
  }

  return imported;
}
