/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
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

#include "profilewizard.h"

#include "profilecheckitem.h"

#include <klineedit.h>
#include <klocale.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

using namespace KSync;

ProfileWizard::ProfileWizard( const Profile &profile,
                              const ActionPartService::List &parts )
  : KDialogBase( Plain, i18n("Configure Profile"), Ok | Cancel, Ok, 0, "wiz" ),
    mProfile( profile ), mAvailableParts( parts )
{
  initUI();
  initProfile();
}

ProfileWizard::ProfileWizard( const ActionPartService::List &parts )
  : KDialogBase( Plain, i18n("Configure Profile"), Ok | Cancel, Ok, 0, "wiz" ),
    mAvailableParts( parts )
{
  initUI();
}

ProfileWizard::~ProfileWizard()
{
}

void ProfileWizard::initUI()
{
  QWidget *topWidget = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topWidget );
  topLayout->setSpacing( spacingHint() );

  QBoxLayout *nameLayout = new QHBoxLayout( topLayout );

  QLabel *label = new QLabel( i18n("Profile name:"), topWidget );
  nameLayout->addWidget( label );

  mNameEdit = new KLineEdit( topWidget );
  nameLayout->addWidget( mNameEdit );

  label = new QLabel( "<qt><b>" + i18n("Which parts to load?") + "</b></qt>", topWidget );
  topLayout->addWidget( label );
  
  label = new QLabel( i18n("KitchenSync supports a variety of plugins. Below\n"
                           "you've the possibility to choose which plugins\n"
                           "should be loaded when this Profile is the active\n"
                           "one."), topWidget );
  topLayout->addWidget( label );

  mPartListView = new KListView( topWidget );
  mPartListView->addColumn( i18n( "Name" ) );
  mPartListView->addColumn( i18n( "Comment" ) );
  mPartListView->setSortColumn( -1 ); // Disable sorting by the user
  mPartListView->setAllColumnsShowFocus( true );
  mPartListView->setFullWidth( true );
  topLayout->addWidget( mPartListView );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  QPushButton *button = new QPushButton( i18n("Add..."), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( addPart() ) );

  button = new QPushButton( i18n("Remove"), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( removePart() ) );

  button = new QPushButton( i18n("Up"), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( raisePart() ) );

  button = new QPushButton( i18n("Down"), topWidget );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( lowerPart() ) );
}

void ProfileWizard::initProfile()
{
  mNameEdit->setText( mProfile.name() );

  ActionPartService::List selectedParts = mProfile.actionParts();
  
  ActionPartService::List::ConstIterator itPart;
  for ( itPart = selectedParts.begin(); itPart != selectedParts.end();
        ++itPart ) {
    new ProfileCheckItem( mPartListView, *itPart );
  }
}

Profile ProfileWizard::profile()
{
  mProfile.setName( mNameEdit->text() );
  mProfile.setActionParts( selectedActionParts() );
  return mProfile;
}

ActionPartService::List ProfileWizard::selectedActionParts()
{
  ActionPartService::List actionParts;
  QListViewItemIterator it( mPartListView );
  for ( ; it.current(); ++it ) {
    ProfileCheckItem *item = static_cast<ProfileCheckItem *>( it.current() );
    actionParts.append( item->actionPart() );
  }
  return actionParts;
}

void ProfileWizard::slotOk()
{
  if ( mNameEdit->text().isEmpty() ) {
    KMessageBox::sorry( this, i18n("Profile name can not be empty.") );
  } else {
    accept();
  }
}

ProfileCheckItem *ProfileWizard::selectedItem()
{
  return static_cast<ProfileCheckItem *>( mPartListView->selectedItem() );
}

void ProfileWizard::addPart()
{
  QStringList partNames;

  ActionPartService::List::ConstIterator it;
  for( it = mAvailableParts.begin(); it != mAvailableParts.end(); ++it ) {
    partNames.append( (*it).name() );
  }

  QString partName = KInputDialog::getItem( i18n("Select Action Part"),
                        i18n("Selection the action part you want to add:"),
                        partNames, 0, false, 0, this );

  for( it = mAvailableParts.begin(); it != mAvailableParts.end(); ++it ) {
    if ( (*it).name() == partName ) {
      ProfileCheckItem *item = selectedItem();
      if ( item ) {
        new ProfileCheckItem( mPartListView, item, *it );
      } else {
        new ProfileCheckItem( mPartListView, *it );
      }
    }
  }
}

void ProfileWizard::removePart()
{
  ProfileCheckItem *item = selectedItem();
  if ( item ) delete item;
}

void ProfileWizard::raisePart()
{
  ProfileCheckItem *item = selectedItem();

  if ( !item )
    return;

  ProfileCheckItem *above;
  above = static_cast<ProfileCheckItem *>( item->itemAbove() );

  if ( above )
    above = static_cast<ProfileCheckItem *>( above->itemAbove() );

  item->moveItem( above );
}

void ProfileWizard::lowerPart()
{
  ProfileCheckItem *item = selectedItem();

  if ( !item )
    return;

  ProfileCheckItem *below;
  below = static_cast<ProfileCheckItem *>( item->nextSibling() );

  if ( below )
    item->moveItem( below );
}

#include "profilewizard.moc"
