
#include "konnectorcheckitem.h"

using namespace KSync;

KonnectorCheckItem::KonnectorCheckItem( QListView* parent,  const KonnectorProfile& prof )
    : QCheckListItem( parent, QString::null, CheckBox),
      m_prof( prof ){
    setText(0, "");
    setText(1, prof.name() );
    if (!prof.udi().isNull() )
        setOn( true );
    m_edit = false;
}
KonnectorCheckItem::~KonnectorCheckItem() {

}
KonnectorProfile KonnectorCheckItem::profile() const {
    return m_prof;
}
bool KonnectorCheckItem::load() const {
    bool load = false;
    if ( m_prof.udi().isNull() &&  isEnabled() ) // not loaded but it's marked as loaded
        load = true;

    return load;
}
bool KonnectorCheckItem::unload() const {
    bool load = false;
    if ( !m_prof.udi().isNull() && !isEnabled() ) // loaded but not marked as loaded unload it now
        load = true;

    return load;
}
bool KonnectorCheckItem::isLoaded() const {
    return (!m_prof.udi().isNull() ); // if udi != null it's loaded
}
bool KonnectorCheckItem::wasEdited()const {
    return m_edit;
}
void KonnectorCheckItem::setEdited( bool b ){
    m_edit = b;
}
void KonnectorCheckItem::setProfile( const KonnectorProfile& prof ){
    m_prof = prof;
}
