/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#include "incidencegeneral.h"

#ifdef KDEPIM_MOBILE_UI
#include <QtGui/QTreeWidgetItem>
#include <KDialog>
#endif

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

using namespace IncidenceEditorsNG;

IncidenceGeneral::IncidenceGeneral( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 )
  , mUi( ui )
{
  mUi->setupUi( this );
 
#ifndef KDEPIM_MOBILE_UI
  mUi->mSecrecyCombo->addItems( KCal::Incidence::secrecyList() );
#endif

#ifdef KDEPIM_MOBILE_UI
//  connect( mUi->mSelectCategoriesButton, SIGNAL(clicked()),
//           SLOT(selectCategories()) );
#else
  connect( mUi->mSecrecyCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()));
#endif
  connect( mUi->mSummaryEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mLocationEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
}

void IncidenceGeneral::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
#ifndef KDEPIM_MOBILE_UI
    Q_ASSERT( mUi->mSecrecyCombo->count() == KCal::Incidence::secrecyList().count() );
    mUi->mSecrecyCombo->setCurrentIndex( mLoadedIncidence->secrecy() );
#endif    
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
  } else {
#ifndef KDEPIM_MOBILE_UI
    mUi->mSecrecyCombo->setCurrentIndex( 0 );
#endif
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
  }

  mWasDirty = false;
}

void IncidenceGeneral::save( KCal::Incidence::Ptr incidence )
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
  incidence->setSecrecy( KCal::Incidence::SecrecyPublic );
#endif
}

bool IncidenceGeneral::isDirty() const
{
#ifndef KDEPIM_MOBILE_UI
  if ( mLoadedIncidence ) {
    if ( mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentIndex() ) {
      return true;
    }
  } else {
    if ( mUi->mSecrecyCombo->currentIndex() != 0 )
      return true;
  }
#endif

  if ( mLoadedIncidence ) {
    return ( mUi->mSummaryEdit->text() != mLoadedIncidence->summary() )
      || ( mUi->mLocationEdit->text() != mLoadedIncidence->location() );
  } else {
    return mUi->mSummaryEdit->text().isEmpty()
      && mUi->mLocationEdit->text().isEmpty();
  }
}

bool IncidenceGeneral::isValid()
{
  if ( mUi->mSummaryEdit->text().isEmpty() ) {
    mUi->mSummaryEdit->setFocus();
    return false;
  }
    
  return true;
}

#include "moc_incidencegeneral.cpp"
