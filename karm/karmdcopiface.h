#ifndef KARM_DCOP_IFAC_H
#define KARM_DCOP_IFAC_H

#include <dcopobject.h>

class KarmDCOPIface : virtual public DCOPObject
{
  K_DCOP
  k_dcop:

  /** Return karm version. */
  virtual QString version() const = 0;
};

#endif // KARM_DCOP_IFAC_H
