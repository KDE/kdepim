
#include "profilecheckitem.h"

using namespace KSync;

ProfileCheckItem::ProfileCheckItem( QListView* parent,
                                    const ManPartService& prof )
    : QCheckListItem( parent, "profile",  CheckBox), m_manpart( prof )
{
    setText(0, m_manpart.name() );
    setText(1, m_manpart.comment() );
}
ProfileCheckItem::~ProfileCheckItem() {
}
ManPartService ProfileCheckItem::manpart()const {
    return m_manpart;
}
