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

#include "incidencewhatwhere.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

#include <KDebug>

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
  kDebug();
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
  } else {
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
  }

  mUi->mLocationEdit->setVisible( type() != KCalCore::Incidence::TypeJournal );
  mUi->label_4->setVisible( type() != KCalCore::Incidence::TypeJournal );

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
  return !mUi->mSummaryEdit->text().isEmpty();
}

QString IncidenceWhatWhere::validate()
{
  if ( mUi->mSummaryEdit->text().isEmpty() ) {
    mUi->mSummaryEdit->setFocus();
    return i18nc( "@info", "Please specify a title." );
  } else {
    return QString();
  }
}

#include "moc_incidencewhatwhere.cpp"
