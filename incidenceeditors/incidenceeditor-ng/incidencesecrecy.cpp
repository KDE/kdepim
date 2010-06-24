/*
    Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "incidencesecrecy.h"

#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

IncidenceSecrecy::IncidenceSecrecy( Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
{
  mUi->mSecrecyCombo->addItems( KCal::Incidence::secrecyList() );

  connect( mUi->mSecrecyCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()));
}

void IncidenceSecrecy::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    Q_ASSERT( mUi->mSecrecyCombo->count() == KCal::Incidence::secrecyList().count() );
    mUi->mSecrecyCombo->setCurrentIndex( mLoadedIncidence->secrecy() );
  } else {
    mUi->mSecrecyCombo->setCurrentIndex( 0 );
  }

  mWasDirty = false;
}

void IncidenceSecrecy::save( KCal::Incidence::Ptr incidence )
{
  Q_ASSERT( incidence );
  incidence->setSummary( mUi->mSummaryEdit->text() );
  incidence->setLocation( mUi->mLocationEdit->text() );

#ifndef KDEPIM_MOBILE_UI
  switch( mUi->mSecrecyCombo->currentIndex() ) {
  case 1:
    incidence->setSecrecy( KCal::Incidence::SecrecyPrivate );
    break;
  case 2:
    incidence->setSecrecy( KCal::Incidence::SecrecyConfidential );
    break;
  default:
    incidence->setSecrecy( KCal::Incidence::SecrecyPublic );
  }
#else
  // ###FIXME
  incidence->setSecrecy( KCal::Incidence::SecrecyPublic );
#endif
}

bool IncidenceSecrecy::isDirty() const
{
  if ( mLoadedIncidence ) {
    if ( mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentIndex() ) {
      return true;
    }
  } else {
    if ( mUi->mSecrecyCombo->currentIndex() != 0 )
      return true;
  }

  return false;
}

