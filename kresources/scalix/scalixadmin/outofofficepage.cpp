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

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextedit.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "jobs.h"
#include "settings.h"

#include "outofofficepage.h"

OutOfOfficePage::OutOfOfficePage( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this, 4, 2, 11, 6 );

  QButtonGroup *group = new QButtonGroup( 1, Qt::Vertical, this );

  mDisabled = new QRadioButton( i18n( "I am in the office" ), group );
  mDisabled->setChecked( true );
  mEnabled = new QRadioButton( i18n( "I am out of the office" ), group );

  mLabel = new QLabel( i18n( "Auto-reply once to each sender with the following text:" ), this );
  mMessage = new QTextEdit( this );
  mSaveButton = new QPushButton( i18n( "Save" ), this );

  layout->addMultiCellWidget( group, 0, 0, 0, 1 );
  layout->addMultiCellWidget( mLabel, 1, 1, 0, 1 );
  layout->addMultiCellWidget( mMessage, 2, 2, 0, 1 );
  layout->addWidget( mSaveButton, 3, 1 );

  statusChanged();

  connect( mEnabled, SIGNAL( toggled( bool ) ), this, SLOT( statusChanged() ) );
  connect( mEnabled, SIGNAL( toggled( bool ) ), this, SLOT( changed() ) );
  connect( mSaveButton, SIGNAL( clicked() ), this, SLOT( store() ) );
  connect( mMessage, SIGNAL( textChanged() ), this, SLOT( changed() ) );

  load();
}

OutOfOfficePage::~OutOfOfficePage()
{
}

void OutOfOfficePage::load()
{
  Scalix::GetOutOfOfficeJob *job = Scalix::getOutOfOffice( Settings::self()->globalSlave(),
                                                           Settings::self()->accountUrl() );
  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( loaded( KIO::Job* ) ) );
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

  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( stored( KIO::Job* ) ) );

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
