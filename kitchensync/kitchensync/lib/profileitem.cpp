
#include "profileitem.h"

using namespace KSync;

ProfileItem::ProfileItem( QListView* parent,  const Profile& prof )
    : QListViewItem(parent ) {
    setProfile( prof );
}
ProfileItem::~ProfileItem() {

}
Profile ProfileItem::profile()const {
    return m_prof;
}
void ProfileItem::setProfile( const Profile& prof ) {
    m_prof = prof;
    setText(0, prof.name() );
}
