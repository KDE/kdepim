#ifndef KSYNCUIKDE_H
#define KSYNCUIKDE_H
// $Id$

class KSyncEntry;

#include "ksyncui.h"

class KSyncUiKde : public KSyncUi
{
  public:
    KSyncUiKde(QWidget *parent);
    virtual ~KSyncUiKde();
    
    KSyncEntry *deconflict(KSyncEntry *syncEntry,KSyncEntry *target);

  private:
    QWidget *mParent;
};

#endif
