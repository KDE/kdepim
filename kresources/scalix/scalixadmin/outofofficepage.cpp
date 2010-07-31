/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <tqbuttongroup.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqtextedit.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "jobs.h"
#include "settings.h"

#include "outofofficepage.h"

OutOfOfficePage::OutOfOfficePage( TQWidget *parent )
  : TQWidget( parent )
{
  TQGridLayout *layout = new TQGridLayout( this, 4, 2, 11, 6 );

  TQButtonGroup *group = new TQButtonGroup( 1, Qt::Vertical, this );

  mDisabled = new TQRadioButton( i18n( "I am in the office" ), group );
  mDisabled->setChecked( true );
  mEnabled = new TQRadioButton( i18n( "I am out of the office" ), group );

  mLabel = new TQLabel( i18n( "Auto-reply once to each sender with the following text:" ), this );
  mMessage = new TQTextEdit( this );
  mSaveButton = new TQPushButton( i18n( "Save" ), this );

  layout->addMultiCellWidget( group, 0, 0, 0, 1 );
  layout->addMultiCellWidget( mLabel, 1, 1, 0, 1 );
  layout->addMultiCellWidget( mMessage, 2, 2, 0, 1 );
  layout->addWidget( mSaveButton, 3, 1 );

  statusChanged();

  connect( mEnabled, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( statusChanged() ) );
  connect( mEnabled, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( changed() ) );
  connect( mSaveButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( store() ) );
  connect( mMessage, TQT_SIGNAL( textChanged() ), this, TQT_SLOT( changed() ) );

  load();
}

OutOfOfficePage::~OutOfOfficePage()
{
}

void OutOfOfficePage::load()
{
  Scalix::GetOutOfOfficeJob *job = Scalix::getOutOfOffice( Settings::self()->globalSlave(),
                                                           Settings::self()->accountUrl() );
  connect( job, TQT_SIGNAL( result( KIO::Job* ) ), TQT_SLOT( loaded( KIO::Job* ) ) );
}

void OutOfOfficePage::loaded( KIO::Job* job )
{
  if ( job->error() ) {
    KMessageBox::error( this, job->errorString() );
    return;
  }

  Scalix::GetOutOfOfficeJob *outOfOfficeJob = static_cast<Scalix::GetOutOfOfficeJob*>( job );

  mEnabled->setChecked( outOfOfficeJob->enabled() );
  mMessage->setText( outOfOfficeJob->message() );

  statusChanged();

  mSaveButton->setEnabled( false );
}

void OutOfOfficePage::store()
{
  Scalix::SetOutOfOfficeJob *job = Scalix::setOutOfOffice( Settings::self()->globalSlave(),
                                                           Settings::self()->accountUrl(),
                                                           mEnabled->isChecked(),
                                                           mMessage->text() );

  connect( job, TQT_SIGNAL( result( KIO::Job* ) ), TQT_SLOT( stored( KIO::Job* ) ) );

  mSaveButton->setEnabled( false );
}

void OutOfOfficePage::stored( KIO::Job* job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
}

void OutOfOfficePage::statusChanged()
{
  bool state = mEnabled->isChecked();

  mLabel->setEnabled( state );
  mMessage->setEnabled( state );
}

void OutOfOfficePage::changed()
{
  mSaveButton->setEnabled( true );
}

#include "outofofficepage.moc"
