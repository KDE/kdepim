
#ifndef KSYNC_KONNECTOR_CHECK_ITEM_H
#define KSYNC_KONNECTOR_CHECK_ITEM_H

#include <qlistview.h>

#include <konnectorprofile.h>

namespace KSync {

    /**
     * A KonnectorCheckItem takes a KonnectorProfile
     * it has a QCheckBox to signalize if it has to be loaded
     * or unloaded. The differences between KonnectorProfile::udi()
     * the state of this item decides if it has to be loaded or unloaded
     */
    class KonnectorCheckItem : public QCheckListItem {
    public:
        /**
         * c'tor the Parent and the Profile
         */
        KonnectorCheckItem( QListView* parent, const KonnectorProfile& prof );
        ~KonnectorCheckItem();

        KonnectorProfile profile()const;

        /**
         * if it has to be loaded
         */
        bool load()const;

        /**
         * if it has to be unloaded
         **/
        bool unload()const;

        /**
         * if the konnector currently is loaded
         */
        bool isLoaded()const;
    private:
        KonnectorProfile m_prof;

    };

};

#endif
