#ifndef KSYNCUIKDE_H
#define KSYNCUIKDE_H
// $Id$


#include "syncui.h"

namespace KSync {

    class SyncEntry;
    class SyncUiKde : public SyncUi
    {
    public:
        SyncUiKde(QWidget *parent, bool confirmDelete, bool inform = FALSE);
        virtual ~SyncUiKde();

        SyncEntry *deconflict(SyncEntry *syncEntry, SyncEntry *target);
        bool confirmDelete( SyncEntry* entry, SyncEntry* target );
        void informBothDeleted( SyncEntry* syncEntry, SyncEntry* target );

    private:
        SyncEntry* deletedChanged( SyncEntry* entry, SyncEntry* target );
        SyncEntry* changedChanged( SyncEntry* entry, SyncEntry* target );

    private:
        QWidget *mParent;
        bool m_confirm : 1;
        bool m_inform  : 1;
    };

}

#endif
