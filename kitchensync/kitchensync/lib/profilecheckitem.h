
#ifndef KSYNC_PROFILE_CHECK_ITEM
#define KSYNC_PROFILE_CHECK_ITEM

#include <qlistview.h>

#include "manpartservice.h"

namespace KSync {
    class ProfileCheckItem : public QCheckListItem {
    public:
        ProfileCheckItem( QListView* item, const ManPartService& );
        ~ProfileCheckItem();
        ManPartService manpart()const;
    private:
        ManPartService m_manpart;
    };
};

#endif
