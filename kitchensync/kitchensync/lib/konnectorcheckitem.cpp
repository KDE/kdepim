
#include "konnectorcheckitem.h"

using namespace KSync;

KonnectorCheckItem::KonnectorCheckItem( QListView* parent,  const KonnectorProfile& prof )
    : QCheckListItem( parent, QString::null, CheckBox),
      m_prof( prof ){
    setText(0, "");
    setText(1, prof.name() );
}
KonnectorCheckItem::~KonnectorCheckItem() {

}
KonnectorProfile KonnectorCheckItem::profile() const {
    return m_prof;
}
bool KonnectorCheckItem::load() const {
    bool load = false;
    if ( m_prof.udi() == QString::null &&  isEnabled() ) // not loaded but it's marked as loaded
        load = true;

    return load;
}
bool KonnectorCheckItem::unload() const {
    bool load = false;
    if ( m_prof.udi() != QString::null && !isEnabled() ) // loaded but not marked as loaded unload it now
        load = true;

    return load;
}
bool KonnectorCheckItem::isLoaded() const {
    return (m_prof.udi() != QString::null ); // if udi != null it's loaded
}
