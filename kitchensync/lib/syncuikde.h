#ifndef KSYNCUIKDE_H
#define KSYNCUIKDE_H
// $Id$


#include "syncui.h"

namespace KSync {

    class SyncEntry;



    class SyncUiKde : public SyncUi
    {
    public:
        SyncUiKde(QWidget *parent);
        virtual ~SyncUiKde();

        SyncEntry *deconflict(SyncEntry *syncEntry, SyncEntry *target);

    private:
        QWidget *mParent;
    };

}

#endif
