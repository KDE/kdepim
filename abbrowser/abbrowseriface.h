#ifndef ABBROWSERIFACE_H
#define ABBROWSERIFACE_H
 
#include <dcopobject.h>

class AbBrowserIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual void addEmail( QString addr ) = 0;
};

#endif
