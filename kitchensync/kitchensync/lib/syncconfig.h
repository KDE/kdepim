#ifndef KSYNC_SYNC_CONFIG_WIDGET_H
#define KSYNC_SYNC_CONFIG_WIDGET_H

#include <qvbox.h>

class QCheckBox;
namespace KSync {
    class SyncConfig : public QVBox {
        Q_OBJECT
    public:
        SyncConfig( bool confirmDelete, bool confirmSync );
        ~SyncConfig();

        bool confirmDelete()const;
        bool confirmSync()const;

    private:
        QCheckBox *m_sync, *m_del;
    };
}


#endif
