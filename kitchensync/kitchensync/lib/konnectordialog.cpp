
#include <klocale.h>
#include <klistview.h>

#include <konnector.h>

#include "konnectordialog.h"
#include "konnectorprofilelistviewbase.h"
#include "konnectorcheckitem.h"

using namespace KSync;

KonnectorDialog::KonnectorDialog( const KonnectorProfile::ValueList& list, KonnectorManager* man )
    : KDialogBase( 0, "KonnectorDialog", true,
                   i18n("Configure Devices"),
                   KDialogBase::Ok|KDialogBase::Cancel,
                   KDialogBase::Ok, true ),
      m_list( list ),  m_manager( man )
{

    m_base = new KonnectorProfileListBase(this);
    m_base->lstView->addColumn("Loaded");
    m_base->lstView->addColumn("Name");

    setMainWidget( m_base );

}
KonnectorDialog::~KonnectorDialog() {

}
KonnectorProfile::ValueList KonnectorDialog::toUnload()const {
    KonnectorProfile::ValueList list;

    return list;
}
KonnectorProfile::ValueList KonnectorDialog::toLoad()const {
    KonnectorProfile::ValueList list;


    return list;
}
KonnectorProfile::ValueList KonnectorDialog::newToLoad()const {

}
KonnectorProfile::ValueList KonnectorDialog::devices()const {

}
/*
 * We know the old ValueList and we can find out the new one
 * So finding deleted items is fairly easy
 */
KonnectorProfile::ValueList KonnectorDialog::removed() const {


}
KonnectorProfile::ValueList KonnectorDialog::list2list() const {


}
void KonnectorDialog::initListView() {
    KonnectorProfile::ValueList::Iterator it;
    for (it = m_list.begin(); it != m_list.end(); ++it ) {
        new KonnectorCheckItem( m_base->lstView, (*it) );
    };
}
