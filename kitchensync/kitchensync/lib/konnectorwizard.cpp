
#include <kcombobox.h>
#include <klocale.h>

#include <konnector.h>

#include "konnectorwizard.h"

#include "konnectorprofilewizardintro.h"
#include "konnectorwizardoutro.h"

using namespace KSync;

KonnectorWizard::KonnectorWizard(KonnectorManager* manager)
    : KWizard(0, "wizard", true), m_manager( manager ) {

    m_free = true;
    initUI();
}
KonnectorWizard::KonnectorWizard(KonnectorManager* manager,
                                 const KonnectorProfile& prof)
    : KWizard(0, "wizard", true ), m_manager( manager) {

    m_free = false;
    initUI();
    initKap();
}
KonnectorWizard::~KonnectorWizard() {

}
KonnectorProfile KonnectorWizard::profile()const {
    KonnectorProfile prof;
    return prof;
}
/*
 * let's add some pages
 * and make the Configure widget be kewl
 * basicly we need to recreate it on
 *
 */
void KonnectorWizard::initUI() {
    m_intro = new KonnectorProfileWizardIntro();
    m_outro = new KonnectorWizardOutro();
    addPage( m_intro, "Profile");
    addPage( m_outro, "Outro");
    setFinishEnabled( m_outro, true );

    Device::ValueList list = m_manager->query();
    Device::ValueList::Iterator it;
    m_intro->cmbDevice->insertItem( i18n("Please choose a Konnector") );
    for (it = list.begin(); it != list.end(); ++it ) {
        m_intro->cmbDevice->insertItem( (*it).identify() );
        m_devices.insert( (*it).identify(), (*it) );

    }
}
void KonnectorWizard::initKap() {


}



#include "konnectorwizard.moc"
