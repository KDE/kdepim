#ifndef KANDYIFACE_H
#define KANDYIFACE_H

#include <dcopobject.h>

class KandyIface : virtual public DCOPObject
{
    K_DCOP
  public:

  k_dcop:
    virtual void importPhonebook() = 0;
};

#endif // KANDYIFACE_H
