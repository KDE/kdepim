#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>

#include "syncconfig.h"

using namespace KSync;

SyncConfig::SyncConfig( bool confirmDelete, bool confirmSync )
    : QVBox()
{
    m_sync = new QCheckBox(i18n("Confirm before writing synced data back.") , this );
    m_sync->setChecked( confirmSync );

    m_del  = new QCheckBox(i18n("Confirm before deleting."), this );
    m_del->setChecked( confirmDelete );
}
SyncConfig::~SyncConfig() {
}
bool SyncConfig::confirmDelete()const {
    return m_del->isChecked();
}
bool SyncConfig::confirmSync()const {
    return m_sync->isChecked();
}
