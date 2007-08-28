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

#include <qlayout.h>
#include <qpushbutton.h>

#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <unistd.h>

#include "delegatedialog.h"
#include "delegateview.h"
#include "jobs.h"
#include "settings.h"

#include "delegatepage.h"

DelegatePage::DelegatePage( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, 11, 6 );

  mView = new DelegateView( &mManager, this );
  layout->addMultiCellWidget( mView, 0, 0, 0, 2 );

  mAddButton = new QPushButton( i18n( "Add Delegate..." ), this );
  layout->addWidget( mAddButton, 1, 0 );

  mEditButton = new QPushButton( i18n( "Edit Delegate..." ), this );
  mEditButton->setEnabled( false );
  layout->addWidget( mEditButton, 1, 1 );

  mRemoveButton = new QPushButton( i18n( "Remove Delegate" ), this );
  mRemoveButton->setEnabled( false );
  layout->addWidget( mRemoveButton, 1, 2 );

  connect( mView, SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( addDelegate() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editDelegate() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeDelegate() ) );

  loadAllDelegates();
}

DelegatePage::~DelegatePage()
{
}

void DelegatePage::loadAllDelegates()
{
  Scalix::GetDelegatesJob *job = Scalix::getDelegates( Settings::self()->globalSlave(),
                                                       Settings::self()->accountUrl() );
  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( allDelegates( KIO::Job* ) ) );
}

void DelegatePage::addDelegate()
{
  DelegateDialog dlg( this );
  dlg.setCaption( i18n( "Add Delegate" ) );

  if ( !dlg.exec() )
    return;

  const Scalix::Delegate delegate = dlg.delegate();

  if ( !delegate.isValid() )
    return;

  Scalix::SetDelegateJob *job = Scalix::setDelegate( Settings::self()->globalSlave(),
                                                     Settings::self()->accountUrl(),
                                                     delegate.email(), delegate.rights() );
  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( delegateAdded( KIO::Job* ) ) );
}

void DelegatePage::editDelegate()
{
  const Scalix::Delegate oldDelegate = mView->selectedDelegate();
  if ( !oldDelegate.isValid() )
    return;

  DelegateDialog dlg( this );
  dlg.setCaption( i18n( "Edit Delegate" ) );

  dlg.setDelegate( oldDelegate );

  if ( !dlg.exec() )
    return;

  const Scalix::Delegate delegate = dlg.delegate();

  if ( !delegate.isValid() )
    return;

  Scalix::SetDelegateJob *job = Scalix::setDelegate( Settings::self()->globalSlave(),
                                                     Settings::self()->accountUrl(),
                                                     delegate.email(), delegate.rights() );
  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( delegateAdded( KIO::Job* ) ) );
}

void DelegatePage::removeDelegate()
{
  const Scalix::Delegate delegate = mView->selectedDelegate();
  if ( !delegate.isValid() )
    return;

  Scalix::DeleteDelegateJob *job = Scalix::deleteDelegate( Settings::self()->globalSlave(),
                                                           Settings::self()->accountUrl(), delegate.email() );
  connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( delegateRemoved( KIO::Job* ) ) );
}

void DelegatePage::allDelegates( KIO::Job *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );

  Scalix::GetDelegatesJob *delegateJob = static_cast<Scalix::GetDelegatesJob*>( job );

  mManager.clear();

  const Scalix::Delegate::List delegates = delegateJob->delegates();
  for ( uint i = 0; i < delegates.count(); ++i )
    mManager.addDelegate( delegates[ i ] );

  selectionChanged();
}

void DelegatePage::delegateAdded( KIO::Job *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
  else
    loadAllDelegates(); // update the GUI
}

void DelegatePage::delegateRemoved( KIO::Job *job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
  else
    loadAllDelegates(); // update the GUI
}

void DelegatePage::selectionChanged()
{
  bool state = ( mView->selectedItem() != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

#include "delegatepage.moc"
