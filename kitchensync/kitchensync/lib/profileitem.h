
#ifndef KSYNC_PROFILE_ITEM_H
#define KSYNC_PROFILE_ITEM_H

#include <qlistview.h>

#include <profile.h>
namespace KSync {
    class ProfileItem : public QListViewItem {
    public:
        ProfileItem( QListView* parent, const Profile& prof );
        ~ProfileItem();
        Profile profile()const;
        void setProfile(const Profile& );
    private:
        Profile m_prof;
    };
};

#endif
