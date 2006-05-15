/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#include <kdeversion.h>
#include <kbuttonbox.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include "resourcecalendar.h"
#include "incidence.h"

#include "selectdialog.h"

using namespace KCal;

SelectDialog::SelectDialog( QPtrList<ResourceCalendar> list,
                            Incidence* incidence,
                            QWidget *parent, const char *name )
  : KDialog( parent, name, true )
{
  setCaption( i18n( "Resource Selection" ) );
  resize( 550, 350 );

  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setMargin( marginHint() );
  mainLayout->setSpacing( 10 );

  // Add a description of the incidence
  QString text = "<b><font size=\"+1\">";
  if ( incidence->type() == "Event" )
    text += i18n( "Choose where you want to store this event" );
  else if ( incidence->type() == "Todo" )
    text += i18n( "Choose where you want to store this task" );
  else
    text += i18n( "Choose where you want to store this incidence" );
  text += "<font></b><br>";
  if ( !incidence->summary().isEmpty() )
    text += i18n( "<b>Summary:</b> %1" ).arg( incidence->summary() ) + "<br>";
  if ( !incidence->location().isEmpty() )
    text += i18n( "<b>Location:</b> %1" ).arg( incidence->location() );
  text += "<br>";
  if ( !incidence->doesFloat() )
    text += i18n( "<b>Start:</b> %1, %2" )
            .arg( incidence->dtStartDateStr(), incidence->dtStartTimeStr() );
  else
    text += i18n( "<b>Start:</b> %1" ).arg( incidence->dtStartDateStr() );
  text += "<br>";
  if ( incidence->type() == "Event" ) {
    Event* event = static_cast<Event*>( incidence );
    if ( event->hasEndDate() )
      if ( !event->doesFloat() )
        text += i18n( "<b>End:</b> %1, %2" )
                .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
      else
        text += i18n( "<b>End:</b> %1" ).arg( event->dtEndDateStr() );
    text += "<br>";
  }
  QLabel *description = new QLabel( text, this, "Incidence description" );
  mainLayout->addWidget( description );

  QGroupBox *groupBox = new QGroupBox( 2, Qt::Horizontal,  this );
  groupBox->setTitle( i18n( "Resources" ) );

  mResourceId = new KListBox( groupBox );

  mainLayout->addWidget( groupBox );

  mainLayout->addSpacing( 10 );

  KButtonBox *buttonBox = new KButtonBox( this );

  buttonBox->addStretch();

#if KDE_IS_VERSION(3,3,0)
  buttonBox->addButton( KStdGuiItem::ok(), this, SLOT( accept() ) );
  buttonBox->addButton( KStdGuiItem::cancel(), this, SLOT( reject() ) );
#else
  buttonBox->addButton( KStdGuiItem::ok().text(), this, SLOT( accept() ) );
  buttonBox->addButton( KStdGuiItem::cancel().text(), this, SLOT( reject() ) );
#endif
    
  buttonBox->layout();

  mainLayout->addWidget( buttonBox );

  // setup listbox
  uint counter = 0;
  for ( uint i = 0; i < list.count(); ++i ) {
    ResourceCalendar *resource = list.at( i );
    if ( resource && !resource->readOnly() ) {
      mResourceMap.insert( counter, resource );
      mResourceId->insertItem( resource->resourceName() );
      counter++;
    }
  }

  mResourceId->setCurrentItem( 0 );
  connect( mResourceId, SIGNAL(returnPressed(QListBoxItem*)),
           SLOT(accept()) );
  connect( mResourceId, SIGNAL( executed( QListBoxItem* ) ),
           SLOT( accept() ) );
}

ResourceCalendar *SelectDialog::resource()
{
  if ( mResourceId->currentItem() != -1 )
    return mResourceMap[ mResourceId->currentItem() ];
  else
    return 0;
}

ResourceCalendar *SelectDialog::getResource( QPtrList<ResourceCalendar> list,
                                             Incidence* incidence,
                                             QWidget *parent )
{
  if ( list.count() == 0 ) {
    KMessageBox::error( parent, i18n( "There is no resource available!" ) );
    return 0;
  }

  if ( list.count() == 1 ) return list.first();

  // the following lines will return a writeable resource if only _one_ writeable
  // resource exists
  ResourceCalendar *found = 0;
  ResourceCalendar *it = list.first();
  while ( it ) {
    if ( !it->readOnly() ) {
      if ( found ) {
        found = 0;
	break;
      } else
        found = it;
    }
    it = list.next();
  }

  if ( found )
    return found;

  SelectDialog dlg( list, incidence, parent );
  if ( dlg.exec() == KDialog::Accepted ) return dlg.resource();
  else return 0;
}
