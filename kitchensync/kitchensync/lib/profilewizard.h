
#ifndef KSYNC_PROFILE_WIZARD_H
#define KSYNC_PROFILE_WIZARD_H

#include <kwizard.h>

#include <profile.h>
#include "manpartservice.h"

class ProfileWizardIntro;
namespace KSync {
    class ProfileWizardChooserImpl;
    class ProfileWizard : public KWizard {
        Q_OBJECT
    public:
        ProfileWizard( const ManPartService::ValueList& );
        ProfileWizard( const Profile& , const ManPartService::ValueList&);
        ~ProfileWizard();
        Profile profile()const;
    private:
        void initUI();
        void init( const ManPartService::ValueList& );
        void initProf(const ManPartService::ValueList& );
        Profile m_prof;
        bool m_useProf:1;
        ProfileWizardChooserImpl *m_choo;
        ProfileWizardIntro *m_intro;
    };
};

#endif
