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
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <KCalUtils/Stringify>

using namespace IncidenceEditorNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceSecrecy::IncidenceSecrecy( Ui::EventOrTodoMore *ui )
#else
IncidenceSecrecy::IncidenceSecrecy( Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui )
{
  setObjectName( "IncidenceSecrecy" );
  mUi->mSecrecyCombo->addItems( KCalUtils::Stringify::secrecyList() );
  connect( mUi->mSecrecyCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()));
}

void IncidenceSecrecy::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    Q_ASSERT( mUi->mSecrecyCombo->count() == KCalUtils::Stringify::secrecyList().count() );
    mUi->mSecrecyCombo->setCurrentIndex( mLoadedIncidence->secrecy() );

    if ( incidence->type() == KCalCore::Incidence::TypeJournal ) {
      mUi->mSecrecyCombo->setVisible( false );
      mUi->mSecrecyLabel->setVisible( false );
    }
  } else {
    mUi->mSecrecyCombo->setCurrentIndex( 0 );
  }

  mWasDirty = false;
}

void IncidenceSecrecy::save( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  switch( mUi->mSecrecyCombo->currentIndex() ) {
  case 1:
    incidence->setSecrecy( KCalCore::Incidence::SecrecyPrivate );
    break;
  case 2:
    incidence->setSecrecy( KCalCore::Incidence::SecrecyConfidential );
    break;
  default:
    incidence->setSecrecy( KCalCore::Incidence::SecrecyPublic );
  }
}

bool IncidenceSecrecy::isDirty() const
{
  if ( mLoadedIncidence ) {
    if ( mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentIndex() ) {
      return true;
    }
  } else {
    if ( mUi->mSecrecyCombo->currentIndex() != 0 ) {
      return true;
    }
  }

  return false;
}

