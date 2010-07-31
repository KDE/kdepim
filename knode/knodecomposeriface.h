#ifndef KNODECOMPOSERIFACE_H
#define KNODECOMPOSERIFACE_H

#include <dcopobject.h>
#include <kurl.h>

class KNodeComposerIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual void initData(const TQString &text) = 0;
};

#endif
