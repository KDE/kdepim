/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>

#include <kdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "freebusywidget.h"

FreeBusyWidget::FreeBusyWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this, KDialog::marginHint(), 
                                         KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Location of Free/Busy information:" ), this );
  layout->addWidget( label );

  mURL = new KURLRequester( this );
  label->setBuddy( mURL );
  layout->addWidget( mURL );

  connect( mURL, SIGNAL( textChanged( const QString& ) ),
           this, SIGNAL( changed() ) );
}

FreeBusyWidget::~FreeBusyWidget()
{
}

void FreeBusyWidget::loadContact( KABC::Addressee *addr )
{
  if ( addr->preferredEmail().isEmpty() )
    return;

  KConfig config( locateLocal( "data", "korganizer/freebusyurls" ) );
  config.setGroup( addr->preferredEmail() );  
  mURL->setURL( config.readEntry( "url" ) );
}

void FreeBusyWidget::storeContact( KABC::Addressee *addr )
{
  if ( addr->preferredEmail().isEmpty() )
    return;

  KConfig config( locateLocal( "data", "korganizer/freebusyurls" ) );
  config.setGroup( addr->preferredEmail() );  
  config.writeEntry( "url", mURL->url() );
  config.sync();
}

void FreeBusyWidget::setReadOnly( bool readOnly )
{
  mURL->setEnabled( !readOnly );
}

#include "freebusywidget.moc"
