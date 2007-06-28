#ifndef _HHDATAPROXY_H
#define _HHDATAPROXY_H


#include "DataProxy.h"

class HHDataProxy : public DataProxy {
  public:
    HHDataProxy();

    void resetSyncFlags();

    virtual void readAppBlock() = 0;

    virtual void writeAppBlock() = 0;


  protected:
    PilotDatabase * fDatabase;

};
#endif
