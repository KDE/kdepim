

#ifndef KSYNC_KONNECTOR_DIALOG_H_CONFIG
#define KSYNC_KONNECTOR_DIALOG_H_CONFIG

#include <qptrlist.h>

#include <kdialogbase.h>

#include <konnectorprofile.h>
#include <konnectorcheckitem.h>

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
         * all devices
         */
        KonnectorProfile::ValueList devices() const;

        /**
         * guess it! Wrong! The removed devices/profiles
         */
        KonnectorProfile::ValueList removed() const;
	
	/**
	 * the edited devices...which are also loaded...
	 */
	KonnectorProfile::ValueList edited()const;

    protected slots:
        virtual void slotRemove();
        virtual void slotAdd();
	virtual void slotEdit();
    private:
        KonnectorProfile::ValueList m_list;
        KonnectorManager* m_manager;
        /** KonnectoCheckItem -> KonnectorProfile::ValueList */
        QPtrList<KonnectorCheckItem> list2list()const;
        KonnectorProfile::ValueList list()const;
        void initListView();
        KonnectorProfileListBase *m_base;

    };
};

#endif
