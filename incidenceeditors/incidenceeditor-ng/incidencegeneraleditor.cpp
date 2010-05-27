/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidencegeneraleditor.h"

#include "categoryconfig.h"
#include "categoryselectdialog.h"
#include "editorconfig.h"

#ifdef KDEPIM_MOBILE_UI
#include "ui_iegeneralmobile.h"
#else
#include "ui_incidencegeneraleditor.h"
#endif

using namespace IncidenceEditors;
using namespace IncidenceEditorsNG;


IncidenceGeneralEditor::IncidenceGeneralEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceGeneralEditor )
{
  mUi->setupUi( this );

#ifndef KDEPIM_MOBILE_UI
  mUi->mSecrecyCombo->addItems( KCal::Incidence::secrecyList() );
  connect( mUi->mSecrecyCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()));
#endif

  connect( mUi->mSelectCategoriesButton, SIGNAL(clicked()),
           SLOT(selectCategories()));
  connect( mUi->mSummaryEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mLocationEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
}

void IncidenceGeneralEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
    setCategories( mLoadedIncidence->categories() );
  } else {
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
    mUi->mCategoriesLabel->clear();
    mSelectedCategories.clear();
  }

  mWasDirty = false;
}

void IncidenceGeneralEditor::save( KCal::Incidence::Ptr incidence )
{
  Q_ASSERT( incidence );
  incidence->setSummary( mUi->mSummaryEdit->text() );
  incidence->setLocation( mUi->mLocationEdit->text() );
  incidence->setCategories( mSelectedCategories );
}

bool IncidenceGeneralEditor::isDirty() const
{
#ifndef KDEPIM_MOBILE_UI
  if ( mLoadedIncidence ) {
    if ( mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentIndex() )
      return true;
  } else {
    if ( mUi->mSecrecyCombo->currentIndex() != 0 )
      return true;
  }
#endif

  if ( mLoadedIncidence ) {
    return ( mUi->mSummaryEdit->text() != mLoadedIncidence->summary() )
      || ( mUi->mLocationEdit->text() != mLoadedIncidence->location() )
      || categoriesChanged();
  } else {
    return mUi->mSummaryEdit->text().isEmpty()
      && mUi->mLocationEdit->text().isEmpty()
      && categoriesChanged();
  }
}

bool IncidenceGeneralEditor::isValid()
{
  if ( mUi->mSummaryEdit->text().isEmpty() ) {
    mUi->mSummaryEdit->setFocus();
    return false;
  }
    
  return true;
}

void IncidenceGeneralEditor::selectCategories()
{
  CategoryConfig cc( EditorConfig::instance()->config() );
  QPointer<CategorySelectDialog> categoryDialog =
    new CategorySelectDialog( &cc );
  categoryDialog->setHelp( "categories-view", "korganizer" );
  categoryDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help );
  categoryDialog->setSelected( mSelectedCategories );

  connect( categoryDialog, SIGNAL(editCategories()),
           SIGNAL(openCategoryDialog()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           categoryDialog, SLOT(updateCategoryConfig()) );

  if ( categoryDialog->exec() )
    setCategories( categoryDialog->selectedCategories() );

  delete categoryDialog;
}

bool IncidenceGeneralEditor::categoriesChanged() const
{
  // If no Incidence was loaded, mSelectedCategories should be empty.
  bool categoriesEqual = mSelectedCategories.isEmpty();

  if ( mLoadedIncidence ) { // There was an Incidence loaded
    categoriesEqual = ( mLoadedIncidence->categories().size() == mSelectedCategories.size() );
    if ( categoriesEqual ) {
      QStringListIterator it( mLoadedIncidence->categories() );
      while ( it.hasNext() && categoriesEqual )
        categoriesEqual = mSelectedCategories.contains( it.next() );
    }
  }
  return !categoriesEqual;
}

void IncidenceGeneralEditor::setCategories( const QStringList &categories )
{
  mSelectedCategories = categories;
  mUi->mCategoriesLabel->setText( categories.join( "," ) );
  checkDirtyStatus();
}

#include "moc_incidencegeneraleditor.cpp"
