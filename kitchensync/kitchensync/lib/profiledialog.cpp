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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

ProfileDialog::ProfileDialog( const Profile::List &profiles,
                              const ActionPartService::List &parts )
  : KDialogBase( Plain, i18n("Configure Profiles"), Ok | Cancel, Ok, 0, 0, true,
                 false ),
    mAvailableParts( parts )
{
  QWidget *topWidget = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topWidget );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( "<qt><b>" + i18n("Setup Profiles") + "</b></qt>", topWidget );
  topLayout->addWidget( label );

  label = new QLabel(
      i18n("A Profile contains information about which Parts\n"
           "should get loaded and used for the synchronization\n"
           "process." ),
      topWidget );
  topLayout->addWidget( label );

  QBoxLayout *listLayout = new QHBoxLayout( topLayout );

  mProfileList = new KListView( topWidget );
  mProfileList->addColumn( i18n( "Name" ) );
  mProfileList->setAllColumnsShowFocus( true );
  mProfileList->setFullWidth( true );
  listLayout->addWidget( mProfileList );  
  connect( mProfileList, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( slotSelectionChanged() ) );

  QBoxLayout *buttonLayout = new QVBoxLayout( listLayout );
  
  QPushButton *button = new QPushButton( i18n("Add..."), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  
  mEditButton = new QPushButton( i18n("Edit..."), topWidget );
  buttonLayout->addWidget( mEditButton );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( slotEdit() ) );
  
  mRemoveButton = new QPushButton( i18n("Remove"), topWidget );
  buttonLayout->addWidget( mRemoveButton );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( slotRemove() ) );
  
  buttonLayout->addStretch();

  initListView( profiles );

  slotSelectionChanged();
}

ProfileDialog::~ProfileDialog()
{
}

void ProfileDialog::initListView( const Profile::List& profiles )
{
  Profile::List::ConstIterator it;
  for ( it = profiles.begin(); it != profiles.end(); ++it ) {
    new ProfileItem( mProfileList, (*it) );
  }
}

Profile::List ProfileDialog::profiles() const
{
  Profile::List profiles;

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
  ProfileWizard wiz( mAvailableParts );

  if ( wiz.exec() ) {
    new ProfileItem( mProfileList, wiz.profile() );
  }
}

void ProfileDialog::slotEdit()
{
  ProfileItem *item =
      static_cast<ProfileItem *>( mProfileList->selectedItem() );
  if ( !item ) return;

  ProfileWizard wiz( item->profile(), mAvailableParts );
  if ( wiz.exec() ) {
    item->setProfile( wiz.profile() );
  }
}

void ProfileDialog::slotSelectionChanged()
{
  bool state = (mProfileList->selectedItem() != 0);

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

#include "profiledialog.moc"
