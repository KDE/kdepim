
#include <klistview.h>
#include <klocale.h>

#include "profilecheckitem.h"
#include "profilewizardchooserimpl.h"


using namespace KSync;

ProfileWizardChooserImpl::ProfileWizardChooserImpl()
    : ProfileWizardChooser( 0 ) {
    KListView3->addColumn( i18n("Name") );
    KListView3->addColumn( i18n("Comment") );
}
ProfileWizardChooserImpl::ProfileWizardChooserImpl( const ManPartService::ValueList& lst,
                                                    QWidget* parent )
    : ProfileWizardChooser( parent ) {
    KListView3->addColumn( i18n("Name") );
    KListView3->addColumn( i18n("Comment") );
    init( lst );
}
ProfileWizardChooserImpl::ProfileWizardChooserImpl( const ManPartService::ValueList& lst1,
                                                    const ManPartService::ValueList& lst2,
                                                    QWidget* parent )
    : ProfileWizardChooser( parent ) {
    KListView3->addColumn( i18n("Name") );
    KListView3->addColumn( i18n("Comment") );
    init( lst1, lst2);
}
ProfileWizardChooserImpl::~ProfileWizardChooserImpl() {

}
void ProfileWizardChooserImpl::init( const ManPartService::ValueList& lst ) {
    initListView( lst );
}
/*
 * Initiliaze from a before configured one. We need to set the items from
 * the lst2 on
 */
void ProfileWizardChooserImpl::init( const ManPartService::ValueList& lst,
                                     const ManPartService::ValueList& lst2 ) {
    ManPartService::ValueList::ConstIterator it1, it2;
    ProfileCheckItem* item;
    for ( it1 =lst.begin(); it1 != lst.end(); ++it1 ) {
        item = new ProfileCheckItem(KListView3, (*it1));
        it2 = lst2.find( (*it1) );
        if ( it2 != lst2.end() )
            item->setOn( true );
    }
}

ManPartService::ValueList ProfileWizardChooserImpl::choosen()const {
    ManPartService::ValueList list;
    QListViewItemIterator it( KListView3 );
    ProfileCheckItem* item;
    for ( ; it.current(); ++it ) {
        item = (ProfileCheckItem*) it.current();
        if ( item && item->isOn() )
            list.append( item->manpart() );
    }
    return list;
}
void ProfileWizardChooserImpl::initListView( const ManPartService::ValueList& lst ) {
    ManPartService::ValueList::ConstIterator it;
    for (it = lst.begin(); it != lst.end(); ++it ) {
        (void)new ProfileCheckItem( KListView3, (*it) );
    }
}

#include "profilewizardchooserimpl.moc"
