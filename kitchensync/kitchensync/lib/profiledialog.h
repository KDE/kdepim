#ifndef KSYNC_PROFILE_DIALOGBASE_H
#define KSYNC_PROFILE_DIALOGBASE_H

#include <kdialogbase.h>

#include <profile.h>
#include "manpartservice.h"

class ProfileListBase;
namespace KSync {
    class ProfileDialog : public KDialogBase {
        Q_OBJECT
    public:
        ProfileDialog( const Profile::ValueList&,
                       const ManPartService::ValueList& );
        ~ProfileDialog();
        Profile::ValueList profiles()const;
    private:
        void initListView( const Profile::ValueList& );
        ManPartService::ValueList m_lst;
        ProfileListBase* m_base;
private slots:
        void slotRemove();
        void slotAdd();
        void slotEdit();
    };
};

#endif
