
#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <kpushbutton.h>

#include <konnectormanager.h>

#include "konnectordialog.h"
#include "konnectorprofilelistviewbase.h"
#include "konnectorcheckitem.h"
#include "konnectorwizard.h"

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

    initListView();
    // connections
    connect( m_base->btnAdd, SIGNAL( clicked() ),
             this, SLOT( slotAdd() ) );
    connect( m_base->btnRemove, SIGNAL( clicked() ),
             this, SLOT( slotRemove() ) );
}
KonnectorDialog::~KonnectorDialog() {
    m_base->lstView->clear();
}
KonnectorProfile::ValueList KonnectorDialog::toUnload()const {
    QPtrList<KonnectorCheckItem> items = list2list();
    KonnectorProfile::ValueList list;
    KonnectorCheckItem* item;
    for ( item = items.first(); item != 0; item = items.next() ) {
        // loaded but not marked as loaded
        if ( !item->isOn() && !item->profile().udi().isNull() )
            list.append( item->profile() );
    }


    return list;
}
KonnectorProfile::ValueList KonnectorDialog::toLoad()const {
    kdDebug(5210) << "toLoad" << endl;
    QPtrList<KonnectorCheckItem> items = list2list();
    KonnectorProfile::ValueList list;
    KonnectorCheckItem* item;
    for ( item = items.first(); item != 0; item = items.next() ) {
        /* not loaded but marked as loaded */
        if ( item->isOn() && item->profile().udi().isNull() ) {
            list.append( item->profile() );
            kdDebug(5210) << " item " << item->profile().name() << endl;
        }
    }


    return list;
}
KonnectorProfile::ValueList KonnectorDialog::devices()const {
    return list();
}
/*
 * We know the old ValueList and we can find out the new one
 * So finding deleted items is fairly easy
 */
KonnectorProfile::ValueList KonnectorDialog::removed() const {
    KonnectorProfile::ValueList lis = list();
    KonnectorProfile::ValueList deleted;
    KonnectorProfile::ValueList::ConstIterator itOld;
    KonnectorProfile::ValueList::ConstIterator itNew;
    bool found = false;

    for ( itOld = m_list.begin(); itOld != m_list.end(); ++itOld ) {
        found = false;
        for ( itNew = lis.begin(); itNew != lis.end(); ++itNew ) {
            if ( (*itNew) == (*itOld) ) {
                found =true;
                break;
            }
        }
        if (!found ) {
            deleted.append( (*itOld) );
        }
    }


    return deleted;
}
QPtrList<KonnectorCheckItem> KonnectorDialog::list2list() const {
    QPtrList<KonnectorCheckItem> list;
    QListViewItemIterator it( m_base->lstView );
    for ( ; it.current(); ++it )
        list.append( ((KonnectorCheckItem*)(it.current()) ) );

    return list;
}
KonnectorProfile::ValueList KonnectorDialog::list() const {
    KonnectorProfile::ValueList list;
    QListViewItemIterator it( m_base->lstView );
    for (;it.current(); ++it )
        list.append( ( (KonnectorCheckItem*)it.current() )->profile() );

    return list;
}
void KonnectorDialog::initListView() {
    KonnectorProfile::ValueList::Iterator it;
    for (it = m_list.begin(); it != m_list.end(); ++it ) {
        new KonnectorCheckItem( m_base->lstView, (*it) );
    };
}
void KonnectorDialog::slotAdd() {
//Wizzard
    KonnectorWizard wiz(m_manager);
    if ( wiz.exec() ) {
        new KonnectorCheckItem( m_base->lstView, wiz.profile() );
    }
}
void KonnectorDialog::slotRemove() {
    KonnectorCheckItem* item = (KonnectorCheckItem*) m_base->lstView->selectedItem();
    if (!item) return;
    m_base->lstView->takeItem( item );
    delete item;
}

#include "konnectordialog.moc"
