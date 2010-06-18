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

#include <QtCore/QDebug>

#ifdef KDEPIM_MOBILE_UI
#include <QtGui/QTreeWidgetItem>
#include <KDialog>
#endif

#include "autochecktreewidget.h"
#include "categoryconfig.h"
#include "categoryhierarchyreader.h"
#include "categoryselectdialog.h"
#include "editorconfig.h"
#include "ui_eventortododialog.h"

using namespace IncidenceEditors;
using namespace IncidenceEditorsNG;


IncidenceGeneralEditor::IncidenceGeneralEditor( Ui::EventOrTodoDestop *ui )
  : IncidenceEditor( 0 )
  , mUi( ui )
{
  qDebug() << this << mUi->mSecrecyCombo->count();
  mUi->mSecrecyCombo->addItems( KCal::Incidence::secrecyList() );
  qDebug() << this << mUi->mSecrecyCombo->count();

#ifdef KDEPIM_MOBILE_UI
//  connect( mUi->mSelectCategoriesButton, SIGNAL(clicked()),
//           SLOT(selectCategories()) );
#else
  CategoryConfig cc( EditorConfig::instance()->config() );
  mUi->mCategoryCombo->setDefaultText( i18nc( "@item:inlistbox", "Select Categories" ) );
  mUi->mCategoryCombo->setSqueezeText( true );
  CategoryHierarchyReaderQComboBox( mUi->mCategoryCombo ).read( cc.customCategories() );

  connect( mUi->mCategoryCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(setCategories(QStringList)) );
#endif
  connect( mUi->mSecrecyCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mSummaryEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mLocationEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
}

void IncidenceGeneralEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    qDebug() << this << mUi->mSecrecyCombo->count();
    Q_ASSERT( mUi->mSecrecyCombo->count() == KCal::Incidence::secrecyList().count() );
    mUi->mSecrecyCombo->setCurrentIndex( mLoadedIncidence->secrecy() );
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
    setCategories( mLoadedIncidence->categories() );
  } else {
    mUi->mSecrecyCombo->setCurrentIndex( 0 );
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
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
}

bool IncidenceGeneralEditor::isDirty() const
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

  qDebug() << "LOADED INCIDENCE" << mLoadedIncidence;
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

void IncidenceGeneralEditor::selectCategories()
{
#ifdef KDEPIM_MOBILE_UI
  CategoryConfig cc( EditorConfig::instance()->config() );
  QPointer<CategorySelectDialog> dialog( new CategorySelectDialog( &cc ) );
  dialog->setSelected( mSelectedCategories );
  dialog->exec();

  setCategories( dialog->selectedCategories() );
  delete dialog;
#endif
}

void IncidenceGeneralEditor::setCategories( const QStringList &categories )
{
  mSelectedCategories = categories;
#ifdef KDEPIM_MOBILE_UI
//  mUi->mCategoriesLabel->setText( mSelectedCategories.join( QLatin1String( "," ) ) );
#endif
  checkDirtyStatus();
}

#include "moc_incidencegeneraleditor.cpp"
