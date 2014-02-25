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
#include "categoryhierarchyreader.h"

#include "categorydialog.h"

#include "editorconfig.h"

#ifdef KDEPIM_MOBILE_UI
  #include "ui_dialogmoremobile.h"
#else
  #include "ui_dialogdesktop.h"
#endif

#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/tagwidgets.h>
#include <KConfigSkeleton>
#include <KDebug>

#include <Akonadi/TagCreateJob>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

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
  //load existing categories
  //TODO this is only for backwards compatiblity and can be removed in 4.14
  checkForUnknownCategories(QStringList());

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
#ifdef KDEPIM_MOBILE_UI
  CategoryConfig cc( EditorConfig::instance()->config() );
  QPointer<CategoryDialog> dialog( new CategoryDialog( &cc ) );
  dialog->setSelected( categories() );
  dialog->exec();

  setCategories( dialog->selectedCategories() );
  delete dialog;
#endif
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
  kDebug() << "mSelectedCategories = " << categories();
  kDebug() << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::checkForUnknownCategories( const QStringList &categoriesToCheck )
{
#ifndef KDEPIM_MOBILE_UI // desktop only
  //TODO CategoryConfig can be removed in the next iteration (4.14), it's only used to migrate existing tags
  CalendarSupport::CategoryConfig cc( EditorConfig::instance()->config() );
  QStringList existingCategories( cc.customCategories() );

  foreach ( const QString &categoryToCheck, categoriesToCheck ) {
    if ( !existingCategories.contains( categoryToCheck ) ) {
      existingCategories.append( categoryToCheck );
    }
  }

  foreach ( const QString &category, existingCategories ) {
    Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(category), this);
    tagCreateJob->setMergeIfExisting(true);
  }
#else
  Q_UNUSED( categoriesToCheck );
#endif
}
