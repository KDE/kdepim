
#ifndef KSYNC_PROFILE_WIZARD_CHOOSER_IMPL_H
#define KSYNC_PROFILE_WIZARD_CHOOSER_IMPL_H

#include "profilewizardchooser.h"
#include "manpartservice.h"

namespace KSync {
    class ProfileWizardChooserImpl : public ProfileWizardChooser {
        Q_OBJECT
    public:
        ProfileWizardChooserImpl();
        ProfileWizardChooserImpl(const ManPartService::ValueList&,
                                 QWidget* parent = 0 );
        /* for editing */
        ProfileWizardChooserImpl(const ManPartService::ValueList&,
                                 const ManPartService::ValueList&,
                                 QWidget* parent = 0 );
        ~ProfileWizardChooserImpl();
        void init( const ManPartService::ValueList& lst );
        void init( const ManPartService::ValueList& lst,
                   const ManPartService::ValueList& lst2 );
        ManPartService::ValueList choosen()const;
    private:
        void initListView( const ManPartService::ValueList& );
    };
};

#endif
