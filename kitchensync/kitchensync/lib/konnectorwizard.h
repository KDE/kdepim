
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
    class ConfigWidget;

    /**
     * The KonnectorWizzard is a KWizard where
     * you can configure a KonnectorProfile
     * including setting the name and creating 
     * a Kapabilities
     * @short a small wizard to create a KonnectorProfile
     */
    class KonnectorWizard : public KWizard {
        Q_OBJECT
    public:
    
	/**
	 * Constructor for creating a new KonnectorProfile
	 * @param manager the KonnectorManager to be used
	 */
        KonnectorWizard( KonnectorManager* manager);
	
	/**
	 * Constructor to edit a KonnectorProfile
	 * @param manager the KonnectorManager to be used
	 * @param prof The KonnectorProfile
	 */
        KonnectorWizard( KonnectorManager* manager, const KonnectorProfile& prof);
        ~KonnectorWizard();
	
	/**
	 * @return the edited KonnectorProfile
	 */
        KonnectorProfile profile() const;
    private:
        void initUI();
        Device byString( const QString&  )const;
        KonnectorManager* m_manager;
        KonnectorProfileWizardIntro *m_intro;
        KonnectorWizardOutro *m_outro;
        ConfigWidget* m_conf;
        QMap<QString, Device> m_devices;
        QString m_current;
	bool m_isEdit;
	Kapabilities m_kaps;

      private slots:
        void slotKonChanged( const QString& );
    };

};


#endif
