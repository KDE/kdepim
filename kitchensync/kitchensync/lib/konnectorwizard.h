
#ifndef KSYNC_KONNECTOR_WIZARD_H
#define KSYNC_KONNECTOR_WIZARD_H

#include <qmap.h> //qt

#include <kwizard.h> //kde

#include <kdevice.h> // libkonnector

#include <konnectorprofile.h> //kitchensyncui

class KonnectorProfileWizardIntro;
class KonnectorWizardOutro;
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
        KonnectorProfileWizardIntro *m_intro;
        KonnectorWizardOutro *m_outro;
        bool m_free:1;
        QMap<QString, Device> m_devices;
    };

};


#endif
