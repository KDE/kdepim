/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <kpushbutton.h>


#include "konnectordialog.h"
#include "konnectorprofilelistviewbase.h"
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
    m_base->lstView->addColumn( i18n( "Activated" ) );
    m_base->lstView->addColumn( i18n( "Name" ) );

    setMainWidget( m_base );

    initListView();
    // connections
    connect( m_base->btnAdd, SIGNAL( clicked() ),
             this, SLOT( slotAdd() ) );
    connect( m_base->btnRemove, SIGNAL( clicked() ),
             this, SLOT( slotRemove() ) );
    connect( m_base->btnEdit, SIGNAL( clicked() ),
	     this, SLOT(slotEdit() ) );
}

KonnectorDialog::~KonnectorDialog()
{
    m_base->lstView->clear();
}

KonnectorProfile::ValueList KonnectorDialog::toUnload() const
{
    QPtrList<KonnectorCheckItem> items = list2list();
    KonnectorProfile::ValueList list;
    KonnectorCheckItem* item;
    for ( item = items.first(); item != 0; item = items.next() ) {
        // loaded but not marked as loaded
        if ( !item->isOn() && item->profile().konnector() )
            list.append( item->profile() );
    }

    return list;
}

KonnectorProfile::ValueList KonnectorDialog::toLoad() const
{
    kdDebug(5210) << "toLoad" << endl;
    QPtrList<KonnectorCheckItem> items = list2list();
    KonnectorProfile::ValueList list;
    KonnectorCheckItem* item;
    for ( item = items.first(); item != 0; item = items.next() ) {
        /* not loaded but marked as loaded */
        if ( item->isOn() && !item->profile().konnector() ) {
            list.append( item->profile() );
            kdDebug(5210) << " item " << item->profile().name() << endl;
        }
    }

    return list;
}

KonnectorProfile::ValueList KonnectorDialog::edited() const
{
    QPtrList<KonnectorCheckItem> items  = list2list();
    KonnectorProfile::ValueList list;
    KonnectorCheckItem* item;
    for( item = items.first(); item != 0; item = items.next() ){
        /* if marked as loaded and is currently loaded... and it is edited... */
	if( item->isOn() && item->profile().konnector() && item->wasEdited() )
		list.append( item->profile() );
    }

    return list;
}

KonnectorProfile::ValueList KonnectorDialog::devices() const
{
    return list();
}

/*
 * We know the old ValueList and we can find out the new one
 * So finding deleted items is fairly easy
 */
KonnectorProfile::ValueList KonnectorDialog::removed() const
{
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

/*
 * converts the list of KonnectorCheckItem
 * into a QPtrList
 */
QPtrList<KonnectorCheckItem> KonnectorDialog::list2list() const
{
    QPtrList<KonnectorCheckItem> list;
    QListViewItemIterator it( m_base->lstView );
    for ( ; it.current(); ++it )
        list.append( ((KonnectorCheckItem*)(it.current()) ) );

    return list;
}

KonnectorProfile::ValueList KonnectorDialog::list() const
{
    KonnectorProfile::ValueList list;
    QListViewItemIterator it( m_base->lstView );
    for (;it.current(); ++it )
        list.append( ( (KonnectorCheckItem*)it.current() )->profile() );

    return list;
}

void KonnectorDialog::initListView()
{
    KonnectorProfile::ValueList::Iterator it;
    for (it = m_list.begin(); it != m_list.end(); ++it ) {
        new KonnectorCheckItem( m_base->lstView, (*it) );
    };
}

void KonnectorDialog::slotAdd()
{
//Wizzard
    KonnectorWizard wiz(m_manager);
    if ( wiz.exec() ) {
        new KonnectorCheckItem( m_base->lstView, wiz.profile() );
    }
}

void KonnectorDialog::slotEdit()
{
    KonnectorCheckItem* item = static_cast<KonnectorCheckItem*> ( m_base->lstView->selectedItem() );
    if(!item ) return;

    KonnectorWizard wiz(m_manager, item->profile() );
    if( wiz.exec() != QDialog::Accepted ) return;

    item->setEdited( true );
    item->setProfile( wiz.profile() );
}

void KonnectorDialog::slotRemove()
{
    KonnectorCheckItem* item = (KonnectorCheckItem*) m_base->lstView->selectedItem();
    if (!item) return;
    m_base->lstView->takeItem( item );
    delete item;
}

#include "konnectordialog.moc"
