#ifndef KNPGP_H
#define KNPGP_H

#include <kpgp.h>

class KNpgp : public Kpgp
{
public:
  KNpgp();
  virtual ~KNpgp();
  virtual void setBusy();
  virtual bool isBusy();
  virtual void idle();
};

#endif
