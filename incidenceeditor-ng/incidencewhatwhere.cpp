/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidencewhatwhere.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmobile.h"
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <KDebug>
#include <KLocalizedString>

using namespace IncidenceEditorNG;

IncidenceWhatWhere::IncidenceWhatWhere( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 ), mUi( ui )
{
  setObjectName( "IncidenceWhatWhere" );
  connect( mUi->mSummaryEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mLocationEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
}

void IncidenceWhatWhere::load( const KCalCore::Incidence::Ptr &incidence )
{
  qDebug();
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
  } else {
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
  }

  mUi->mLocationEdit->setVisible( type() != KCalCore::Incidence::TypeJournal );
  mUi->mLocationLabel->setVisible( type() != KCalCore::Incidence::TypeJournal );

  mWasDirty = false;
}

void IncidenceWhatWhere::save( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  incidence->setSummary( mUi->mSummaryEdit->text() );
  incidence->setLocation( mUi->mLocationEdit->text() );
}

bool IncidenceWhatWhere::isDirty() const
{
  if ( mLoadedIncidence ) {
    return
      ( mUi->mSummaryEdit->text() != mLoadedIncidence->summary() ) ||
      ( mUi->mLocationEdit->text() != mLoadedIncidence->location() );
  } else {
    return
      mUi->mSummaryEdit->text().isEmpty() &&
      mUi->mLocationEdit->text().isEmpty();
  }
}

bool IncidenceWhatWhere::isValid() const
{
  if ( mUi->mSummaryEdit->text().isEmpty() ) {
    qDebug() << "Specify a title";
    mLastErrorString = i18nc( "@info", "Please specify a title." );
    return false;
  } else {
    mLastErrorString.clear();
    return true;
  }
}

void IncidenceWhatWhere::validate()
{
  if ( mUi->mSummaryEdit->text().isEmpty() ) {
    mUi->mSummaryEdit->setFocus();
  }
}

