
#include <konnector.h>

#include "konnectorwizard.h"

#include "konnectorprofilewizardintro.h"
#include "konnectorwizardoutro.h"

using namespace KSync;

KonnectorWizard::KonnectorWizard(KonnectorManager* manager)
    : KWizard(0, "wizard", true), m_manager( manager ) {

    initUI();
}
KonnectorWizard::KonnectorWizard(KonnectorManager* manager,
                                 const KonnectorProfile& prof)
    : KWizard(0, "wizard", true ), m_manager( manager) {

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
    addPage( new KonnectorProfileWizardIntro(), "Profile");
    addPage( new KonnectorWizardOutro(), "Outro");
}
void KonnectorWizard::initKap() {


}



#include "konnectorwizard.moc"
