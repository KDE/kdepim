/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "profiledialog.h"

#include "profileitem.h"
#include "profilewizard.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

using namespace KSync;

ProfileDialog::ProfileDialog( const Profile::ValueList& profiles,
                              const ManPartService::ValueList& man )
  : KDialogBase( Plain, i18n("Configure Profiles"), Ok | Cancel, Ok, 0, 0, true,
                 false ),
    m_lst( man )
{
  QWidget *topWidget = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topWidget );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("Setup Profiles"), topWidget );
  topLayout->addWidget( label );

  label = new QLabel(
      i18n("A Profile contains information about which Parts\n"
           "should get loaded and used for the synchronization\n"
           "process." ),
      topWidget );
  topLayout->addWidget( label );

  QBoxLayout *listLayout = new QHBoxLayout( topLayout );

  mProfileList = new KListView( topWidget );
  mProfileList->addColumn( i18n("Name") );
  listLayout->addWidget( mProfileList );  

  QBoxLayout *buttonLayout = new QVBoxLayout( listLayout );
  
  QPushButton *button = new QPushButton( i18n("Add..."), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  
  button = new QPushButton( i18n("Edit..."), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotEdit() ) );
  
  button = new QPushButton( i18n("Remove"), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotRemove() ) );
  
  buttonLayout->addStretch();

  initListView( profiles );
}

ProfileDialog::~ProfileDialog()
{
}

void ProfileDialog::initListView( const Profile::ValueList& profiles )
{
  Profile::ValueList::ConstIterator it;
  for ( it = profiles.begin(); it != profiles.end(); ++it ) {
    new ProfileItem( mProfileList, (*it) );
  }
}

Profile::ValueList ProfileDialog::profiles() const
{
  Profile::ValueList profiles;

  QListViewItemIterator it( mProfileList );
  for ( ; it.current(); ++it ) {
    ProfileItem *item = static_cast<ProfileItem *>( it.current() );
    profiles.append( item->profile() );
  }

  return profiles;
}

void ProfileDialog::slotRemove()
{
  delete mProfileList->selectedItem();
}

void ProfileDialog::slotAdd()
{
  ProfileWizard wiz( m_lst );

  if ( wiz.exec() ) {
    new ProfileItem( mProfileList, wiz.profile() );
  }
}

void ProfileDialog::slotEdit()
{
  ProfileItem *item =
      static_cast<ProfileItem *>( mProfileList->selectedItem() );
  if ( !item ) return;

  ProfileWizard wiz( item->profile(), m_lst );
  if ( wiz.exec() ) {
    item->setProfile( wiz.profile() );
  }
}

#include "profiledialog.moc"
