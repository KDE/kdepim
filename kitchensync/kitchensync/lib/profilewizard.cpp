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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "profilewizard.h"

#include "profilecheckitem.h"

#include <klineedit.h>
#include <klocale.h>
#include <klistview.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qlabel.h>

using namespace KSync;

ProfileWizard::ProfileWizard( const Profile &profile,
                              const ManPartService::ValueList &parts )
  : KDialogBase( Plain, i18n("Configure Profile"), Ok | Cancel, Ok, 0, "wiz" ),
    mProfile( profile )
{
  initUI();
  initPartList( parts );
  initProfile();
}

ProfileWizard::ProfileWizard( const ManPartService::ValueList &parts )
  : KDialogBase( Plain, i18n("Configure Profile"), Ok | Cancel, Ok, 0, "wiz" )
{
  initUI();
  initPartList( parts );
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

  QLabel *label = new QLabel( i18n("Profile Name"), topWidget );
  nameLayout->addWidget( label );

  mNameEdit = new KLineEdit( topWidget );
  nameLayout->addWidget( mNameEdit );

  label = new QLabel( i18n("Which Parts to load?"), topWidget );
  topLayout->addWidget( label );
  
  label = new QLabel( i18n("KitchenSync supports a variety of plugins. Below\n"
                           "you've the possibility to choose which plugins\n"
                           "should be loaded when this Profile is the active\n"
                           "one."), topWidget );
  topLayout->addWidget( label );

  mPartListView = new KListView( topWidget );
  mPartListView->addColumn( i18n("Name") );
  mPartListView->addColumn( i18n("Comment") );
  topLayout->addWidget( mPartListView );
}

void ProfileWizard::initPartList( const ManPartService::ValueList &parts )
{
  ManPartService::ValueList::ConstIterator it;
  for ( it = parts.begin(); it != parts.end(); ++it ) {
    new ProfileCheckItem( mPartListView, *it );
  }
}

void ProfileWizard::initProfile()
{
  mNameEdit->setText( mProfile.name() );

  ManPartService::ValueList selectedParts = mProfile.manParts();
  
  ManPartService::ValueList::ConstIterator itPart;
  for ( itPart = selectedParts.begin(); itPart != selectedParts.end();
        ++itPart ) {
    QListViewItemIterator it( mPartListView );
    for ( ; it.current(); ++it ) {
      ProfileCheckItem *item = static_cast<ProfileCheckItem *>( it.current() );
      if ( item->manpart() == *itPart ) {
        item->setOn( true );
        break;
      }
    }
  }
}

Profile ProfileWizard::profile()
{
  mProfile.setName( mNameEdit->text() );
  mProfile.setManParts( selectedManParts() );
  return mProfile;
}

ManPartService::ValueList ProfileWizard::selectedManParts()
{
  ManPartService::ValueList manparts;
  QListViewItemIterator it( mPartListView );
  for ( ; it.current(); ++it ) {
    ProfileCheckItem *item = static_cast<ProfileCheckItem *>( it.current() );
    if ( item->isOn() ) manparts.append( item->manpart() );
  }
  return manparts;
}

void ProfileWizard::slotOk()
{
  if ( mNameEdit->text().isEmpty() ) {
    KMessageBox::sorry( this, i18n("Profile name can not be empty.") );
  } else {
    accept();
  }
}

#include "profilewizard.moc"
