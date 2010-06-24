#include "incidencecategories.h"

#include "autochecktreewidget.h"
#include "categoryconfig.h"
#include "categoryhierarchyreader.h"
#include "categoryselectdialog.h"
#include "editorconfig.h"

#include "ui_eventortododesktop.h"

using namespace IncidenceEditors;
using namespace IncidenceEditorsNG;

IncidenceCategories::IncidenceCategories( Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
{
  CategoryConfig cc( EditorConfig::instance()->config() );
  mUi->mCategoryCombo->setDefaultText( i18nc( "@item:inlistbox", "Select Categories" ) );
  mUi->mCategoryCombo->setSqueezeText( true );
  CategoryHierarchyReaderQComboBox( mUi->mCategoryCombo ).read( cc.customCategories() );

  connect( mUi->mCategoryCombo, SIGNAL(checkedItemsChanged(QStringList)),
           SLOT(setCategories(QStringList)) );
}

void IncidenceCategories::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence )
    setCategories( mLoadedIncidence->categories() );
  else
    mSelectedCategories.clear();

  mWasDirty = false;
}

void IncidenceCategories::save( KCal::Incidence::Ptr incidence )
{
  Q_ASSERT( incidence );
  incidence->setCategories( mSelectedCategories );
}

bool IncidenceCategories::isDirty() const
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

void IncidenceCategories::selectCategories()
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

void IncidenceCategories::setCategories( const QStringList &categories )
{
  mSelectedCategories = categories;
#ifdef KDEPIM_MOBILE_UI
//  mUi->mCategoriesLabel->setText( mSelectedCategories.join( QLatin1String( "," ) ) );
#endif
  checkDirtyStatus();
}

