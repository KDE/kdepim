/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>


#include "profilelistbase.h"
#include "profileitem.h"
#include "profilewizard.h"
#include "profiledialog.h"

using namespace KSync;

ProfileDialog::ProfileDialog( const Profile::ValueList& lst,
                              const ManPartService::ValueList& man )
    : KDialogBase(0, "ProfileDialog", true,
                  i18n("Configure Profiles"),
                  KDialogBase::Ok|KDialogBase::Cancel,
                  KDialogBase::Ok, true ),
      m_lst( man) {
    m_base = new ProfileListBase( this );
    m_base->lstEdit->addColumn( "Name");
    setMainWidget( m_base );
    initListView( lst );
    connect(m_base->btnAdd, SIGNAL( clicked() ),
            this, SLOT( slotAdd() ) );
    connect(m_base->btnEdit, SIGNAL( clicked() ),
            this, SLOT(slotEdit() ) );
    connect(m_base->btnRemove, SIGNAL(clicked() ),
            this, SLOT(slotRemove() ) );
}
ProfileDialog::~ProfileDialog() {

}
/*
 * go through the list and add the items
 */
Profile::ValueList ProfileDialog::profiles()const {
    Profile::ValueList lst;
    ProfileItem* item;

    QListViewItemIterator it( m_base->lstEdit );
    for (; it.current(); ++it ) {
        item = (ProfileItem*) it.current();
        lst.append( item->profile() );
    }

    return lst;
}
/*
 * init the Profiles
 */
void ProfileDialog::initListView( const Profile::ValueList& lst ) {
    Profile::ValueList::ConstIterator it;

    for (it =lst.begin(); it!= lst.end(); ++it ) {
        (void)new ProfileItem( m_base->lstEdit, (*it) );
    }
}
void ProfileDialog::slotRemove() {
    ProfileItem* item = (ProfileItem*)m_base->lstEdit->selectedItem();
    if ( !item ) return;

    m_base->lstEdit->takeItem( item );
    delete item;
}
void ProfileDialog::slotAdd() {
    ProfileWizard wiz( m_lst );

    if ( wiz.exec() ) {
        (void)new ProfileItem( m_base->lstEdit, wiz.profile() );
    }
}
void ProfileDialog::slotEdit() {
    ProfileItem* item = (ProfileItem*)m_base->lstEdit->selectedItem();
    if ( !item ) return;

    ProfileWizard wiz( item->profile(), m_lst );
    if (wiz.exec() ) {
        item->setProfile( wiz.profile() );
    }
}

#include "profiledialog.moc"
