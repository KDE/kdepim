
#ifndef KSYNC_KONNECTOR_WIZARD_H
#define KSYNC_KONNECTOR_WIZARD_H

#include <kwizard.h>

#include <konnectorprofile.h>

namespace KSync{
    class KonnectorManager;
    class KonnectorWizard : public KWizard {
        Q_OBJECT
    public:
        KonnectorWizard( KonnectorManager* manager);
        KonnectorWizard( KonnectorManager*, const KonnectorProfile& );
        ~KonnectorWizard();
        KonnectorProfile profile() const;
    private:
        void initUI();
        void initKap();
        KonnectorManager* m_manager;
    };

};


#endif
