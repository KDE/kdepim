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

#include "incidencecategories.h"

#include "editorconfig.h"

#ifdef KDEPIM_MOBILE_UI
  #include "ui_dialogmoremobile.h"
#else
  #include "ui_dialogdesktop.h"
#endif

#include <libkdepim/widgets/tagwidgets.h>
#include <QDebug>

#include <TagCreateJob>

using namespace IncidenceEditorNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceCategories::IncidenceCategories( Ui::EventOrTodoMore *ui )
#else
IncidenceCategories::IncidenceCategories( Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui )
{
  setObjectName( "IncidenceCategories" );

#ifdef KDEPIM_MOBILE_UI
connect( mUi->mSelectCategoriesButton, SIGNAL(clicked()),
          SLOT(selectCategories()) );
#else
  connect( mUi->mTagWidget, SIGNAL(selectionChanged(QStringList)),
          SLOT(onSelectionChanged(QStringList)) );
#endif
}

void IncidenceCategories::onSelectionChanged(const QStringList &list)
{
  mSelectedCategories = list;
  checkDirtyStatus();
}

void IncidenceCategories::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    checkForUnknownCategories( mLoadedIncidence->categories() );
    setCategories( mLoadedIncidence->categories() );
  } else {
    setCategories( QStringList() );
  }

  mWasDirty = false;
}

void IncidenceCategories::save( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  incidence->setCategories( categories() );
}

QStringList IncidenceCategories::categories() const
{
  return mSelectedCategories;
}

bool IncidenceCategories::isDirty() const
{
  // If no Incidence was loaded, mSelectedCategories should be empty.
  bool categoriesEqual = categories().isEmpty();

  if ( mLoadedIncidence ) { // There was an Incidence loaded
    categoriesEqual =
      ( mLoadedIncidence->categories().toSet() == categories().toSet() );
  }
  return !categoriesEqual;
}

void IncidenceCategories::selectCategories()
{
  QPointer<KPIM::TagSelectionDialog> dialog( new KPIM::TagSelectionDialog() );
  dialog->setSelection( categories() );
  dialog->exec();

  if (dialog) {
    setCategories( dialog->selection() );
    delete dialog;
  }
}

void IncidenceCategories::setCategories( const QStringList &categories )
{
  mSelectedCategories = categories;
#ifdef KDEPIM_MOBILE_UI
  mUi->mCategoriesLabel->setText( mSelectedCategories.join( QLatin1String( "," ) ) );
#else
  mUi->mTagWidget->setSelection(categories);
#endif

  checkDirtyStatus();
}

void IncidenceCategories::printDebugInfo() const
{
  qDebug() << "mSelectedCategories = " << categories();
  qDebug() << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::checkForUnknownCategories( const QStringList &categoriesToCheck )
{
#ifndef KDEPIM_MOBILE_UI // desktop only
  foreach ( const QString &category, categoriesToCheck ) {
    Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(category), this);
    tagCreateJob->setMergeIfExisting(true);
  }
#else
  Q_UNUSED( categoriesToCheck );
#endif
}
