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
#include <KConfigSkeleton>
#include <KDebug>

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
  CategoryConfig cc( EditorConfig::instance()->config() );
  mUi->mCategoryCombo->setDefaultText( i18nc( "@item:inlistbox", "Select Categories" ) );
  mUi->mCategoryCombo->setSqueezeText( true );
  CategoryHierarchyReaderQComboBox( mUi->mCategoryCombo ).read( cc.customCategories() );

  connect( mUi->mCategoryCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(setCategoriesFromCombo()) );
#endif
}

void IncidenceCategories::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
#ifdef KDEPIM_MOBILE_UI
    setCategories( mLoadedIncidence->categories() );
#else
    checkForUnknownCategories( mLoadedIncidence->categories() );
    mUi->mCategoryCombo->setCheckedItems( mLoadedIncidence->categories(), Qt::UserRole );
#endif
  } else {
    mSelectedCategories.clear();
  }

  mWasDirty = false;
}

void IncidenceCategories::save( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  incidence->setCategories( mSelectedCategories );
}

QStringList IncidenceCategories::categories() const
{
  return mSelectedCategories;
}

bool IncidenceCategories::isDirty() const
{
  // If no Incidence was loaded, mSelectedCategories should be empty.
  bool categoriesEqual = mSelectedCategories.isEmpty();

  if ( mLoadedIncidence ) { // There was an Incidence loaded
    categoriesEqual =
      ( mLoadedIncidence->categories().toSet().size() == mSelectedCategories.toSet().size() );

    if ( categoriesEqual ) {
      QStringListIterator it( mLoadedIncidence->categories() );
      while ( it.hasNext() && categoriesEqual ) {
        categoriesEqual = mSelectedCategories.contains( it.next() );
      }
    }
  }
  return !categoriesEqual;
}

void IncidenceCategories::selectCategories()
{
#ifdef KDEPIM_MOBILE_UI
  CategoryConfig cc( EditorConfig::instance()->config() );
  QPointer<CategoryDialog> dialog( new CategoryDialog( &cc ) );
  dialog->setSelected( mSelectedCategories );
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
#endif

  checkDirtyStatus();
}

void IncidenceCategories::setCategoriesFromCombo()
{
#ifndef KDEPIM_MOBILE_UI
  const QStringList categories = mUi->mCategoryCombo->checkedItems( Qt::UserRole );
  setCategories( categories );
#endif
}

void IncidenceCategories::printDebugInfo() const
{
  kDebug() << "mSelectedCategories = " << mSelectedCategories;
  kDebug() << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::checkForUnknownCategories( const QStringList &categoriesToCheck )
{
#ifndef KDEPIM_MOBILE_UI // desktop only
  CalendarSupport::CategoryConfig cc( EditorConfig::instance()->config() );

  QStringList existingCategories( cc.customCategories() );
  bool found = false;
  foreach ( const QString &categoryToCheck, categoriesToCheck ) {
    if ( !existingCategories.contains( categoryToCheck ) ) {
      existingCategories.append( categoryToCheck );
      found = true;
    }
  }

  cc.setCustomCategories( existingCategories );
  cc.writeConfig();

  if ( found ) {
    //update the combo with newly found categories
    CategoryHierarchyReaderQComboBox( mUi->mCategoryCombo ).read( existingCategories );
  }
#else
  Q_UNUSED( categoriesToCheck )
#endif
}

