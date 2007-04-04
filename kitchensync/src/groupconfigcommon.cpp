/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/


#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <libqopensync/group.h>
#include <libqopensync/conversion.h>
#include <libqopensync/environment.h>

#include "syncprocess.h"
#include "syncprocessmanager.h"

#include "groupconfigcommon.h"

ObjectTypeSelector::ObjectTypeSelector( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 0 );

  const QSync::Conversion conversion = SyncProcessManager::self()->environment()->conversion();

  QMap<QString, QString> objectTypeMap;
  objectTypeMap.insert( "contact", i18n( "Contacts" ) );
  objectTypeMap.insert( "event", i18n( "Events" ) );
  objectTypeMap.insert( "todo", i18n( "To-dos" ) );
  objectTypeMap.insert( "note", i18n( "Notes" ) );

  QStringList objectTypes = conversion.objectTypes();

  // reorder the entries so that contact and event are the first one
  qHeapSort( objectTypes );

  QStringList reoderedObjectTypes, stack;
  for ( uint i = 0; i < objectTypes.count(); ++i ) {
    if ( objectTypes[ i ] == "contact" || objectTypes[ i ] == "event" )
      reoderedObjectTypes.append( objectTypes[ i ] );
    else
      stack.append( objectTypes[ i ] );
  }
  reoderedObjectTypes += stack;

  QStringList::ConstIterator it;

  int row = 0;
  int col = 0;
  for( it = reoderedObjectTypes.begin(); it != reoderedObjectTypes.end(); ++it ) {
    QString objectType = *it;

    // Don't display object type "data". Object type "data" is a kind of wildcard - so don't filter * 
    if ( objectType == "data" )
      continue;

    QCheckBox *objectCheckBox = new QCheckBox( objectTypeMap[ objectType ], this );
    layout->addWidget( objectCheckBox, row, col );
    mObjectTypeChecks.insert( objectType, objectCheckBox );

    col++;
    if ( (row == 0 && col == 2) || col == 3 ) {
      col = 0;
      row++;
    }
  }
}

void ObjectTypeSelector::load( const QSync::Group &group )
{
  const QSync::GroupConfig config = group.config();

  const QStringList objectTypes = config.activeObjectTypes();

  // Enable everything on the inital load
  bool initialLoad = false;
  if ( objectTypes.isEmpty() )
    initialLoad = true;

  QMap<QString, QCheckBox*>::ConstIterator it;
  for( it = mObjectTypeChecks.begin(); it != mObjectTypeChecks.end(); ++it ) {
    QCheckBox *check = it.data();
    check->setChecked( objectTypes.contains( it.key() ) || initialLoad );
  }
}

void ObjectTypeSelector::save( QSync::Group group )
{
  QStringList objectTypes;

  QMap<QString,QCheckBox *>::ConstIterator it;
  for( it = mObjectTypeChecks.begin(); it != mObjectTypeChecks.end(); ++it ) {
    QCheckBox *check = it.data();
    if ( check->isChecked() )
      objectTypes.append( it.key() );
  }

  // Always add object type "data"
  objectTypes.append( "data" );

  QSync::GroupConfig config = group.config();
  config.setActiveObjectTypes( objectTypes );
}

GroupConfigCommon::GroupConfigCommon( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this, 2, 2, KDialog::marginHint(), KDialog::spacingHint() );

  layout->addWidget( new QLabel( i18n( "Name:" ), this ), 0, 0 );

  mGroupName = new KLineEdit( this );
  layout->addWidget( mGroupName, 0, 1 );

  layout->addWidget( new QLabel( i18n( "Object Types to be Synchronized:"), this ), 1, 0, Qt::AlignTop );

  mObjectTypeSelector = new ObjectTypeSelector( this );
  layout->addWidget( mObjectTypeSelector, 1, 1 );

  layout->setRowStretch( 2, 1 );
}

void GroupConfigCommon::setSyncProcess( SyncProcess *syncProcess )
{
  mSyncProcess = syncProcess;

  mGroupName->setText( mSyncProcess->group().name() );
  mObjectTypeSelector->load( mSyncProcess->group() );
}

void GroupConfigCommon::save()
{
  mSyncProcess->group().setName( mGroupName->text() );
  mObjectTypeSelector->save( mSyncProcess->group() );
}
