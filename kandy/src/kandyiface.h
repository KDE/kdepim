#ifndef KANDYIFACE_H
#define KANDYIFACE_H

#include <dcopobject.h>

class KandyIface : virtual public DCOPObject
{
    K_DCOP
  public:

  k_dcop:
    virtual void syncPhonebooks() = 0;
    virtual void exit() = 0;
};

#endif // KANDYIFACE_H
