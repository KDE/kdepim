

#ifndef KSYNC_KONNECTOR_DIALOG_H_CONFIG
#define KSYNC_KONNECTOR_DIALOG_H_CONFIG

#include <kdialogbase.h>

#include <konnectorprofile.h>

class KonnectorProfileListBase;
namespace KSync {
    class KonnectorManager;
    /**
     * KonnectorDialog is the configure dialog to for the
     * user to load, unload, add and remove KonnectorProfiles
     * It starts the Wizzard and gives informations about which
     * konnectors to load and unload
     */
    class KonnectorDialog : public KDialogBase {
        Q_OBJECT
    public:
        /**
         * it's modal and don't need a parent
         */
        KonnectorDialog( const KonnectorProfile::ValueList& , KonnectorManager* man);
        ~KonnectorDialog();

        /**
         * devices which needs to be unloaded
         */
        KonnectorProfile::ValueList toUnload() const;

        /**
         * devices which need to be loaded
         */
        KonnectorProfile::ValueList toLoad() const;

        /**
         * new devices to load
         */
        KonnectorProfile::ValueList newToLoad() const;

        /**
         * all devices
         */
        KonnectorProfile::ValueList devices() const;

        /**
         * guess it! Wrong! The removed devices/profiles
         */
        KonnectorProfile::ValueList removed() const;

    private:
        KonnectorProfile::ValueList m_list;
        KonnectorManager* m_manager;
        /** KonnectoCheckItem -> KonnectorProfile::ValueList */
        KonnectorProfile::ValueList list2list()const;
        void initListView();
        KonnectorProfileListBase *m_base;

    };
};

#endif
