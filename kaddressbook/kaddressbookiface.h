#ifndef KADDRESSBOOKIFACE_H
#define KADDRESSBOOKIFACE_H
 
#include <dcopobject.h>
#include <qstringlist.h>

class KAddressBookIface : virtual public DCOPObject
{
  K_DCOP

  k_dcop:
    virtual void addEmail( QString addr ) = 0;
  
    virtual ASYNC showContactEditor( QString uid ) = 0;

    /**
      Show's dialog for creation of a new contact.  Returns once a contact
      is created (or canceled).
     */
    virtual void newContact() = 0;

    /**
      Save changes to the address book files.
     */
    virtual QString getNameByPhone( QString phone ) = 0;
    virtual void save() = 0;
    virtual void exit() = 0;
};

#endif
