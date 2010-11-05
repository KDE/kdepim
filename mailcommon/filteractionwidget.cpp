/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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


#include "filteractionwidget.h"
#include "filteraction.h"
#include "mailkernel.h"
#include "mailfilter.h"
#include "minimumcombobox.h"

#include <KLocalizedString>

#include <QGridLayout>
#include <QLabel>

#include <assert.h>



using namespace MailCommon;

//=============================================================================
//
// class FilterActionWidget
//
//=============================================================================

FilterActionWidget::FilterActionWidget( QWidget *parent, const char* name )
  : KHBox( parent )
{
  setObjectName( name );

  int i;

  QWidget *w = new QWidget( this );
  gl = new QGridLayout( w );
  gl->setContentsMargins( 0, 0, 0, 0 );
  mComboBox = new MinimumComboBox( w );
  mComboBox->setEditable( false );
  assert( mComboBox );
  gl->addWidget( mComboBox, 1, 1 );

  setSpacing( 4 );

  QList<FilterActionDesc*> list = FilterIf->filterActionDict()->list();
  QList<FilterActionDesc*>::const_iterator it;
  for ( i=0, it = list.constBegin() ; it != list.constEnd() ; ++it, ++i ) {
    //create an instance:
    FilterAction *a = (*it)->create();
    // append to the list of actions:
    mActionList.append( a );
    // add (i18n-ized) name to combo box
    mComboBox->addItem( (*it)->label );
  }
  // widget for the case where no action is selected.
  mComboBox->addItem( " " );
  mComboBox->setCurrentIndex(i);

  // don't show scroll bars.
  mComboBox->setMaxCount( mComboBox->count() );
  // layout management:
  // o the combo box is not to be made larger than it's sizeHint(),
  //   the parameter widget should grow instead.
  // o the whole widget takes all space horizontally, but is fixed vertically.
  mComboBox->adjustSize();
  mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  updateGeometry();

  // redirect focus to the filter action combo box
  setFocusProxy( mComboBox );

  // now connect the combo box and the widget stack
  connect( mComboBox, SIGNAL( activated( int ) ),
          this, SLOT( slotFilterTypeChanged( int ) ) );

  setFilterAction();
}

void FilterActionWidget::setFilterAction( QWidget* w )
{
  if ( gl->itemAtPosition( 1, 2 ) )
    delete gl->itemAtPosition( 1, 2 )->widget();

  if ( w )
    gl->addWidget( w, 1, 2/*, Qt::AlignTop*/ );
  else
    gl->addWidget( new QLabel( i18n( "Please select an action." ), this ), 1, 2 );
}

void FilterActionWidget::slotFilterTypeChanged( int newIdx )
{
  setFilterAction( newIdx < mActionList.count() ? mActionList.at( newIdx )->createParamWidget( this ) : 0 );
}

FilterActionWidget::~FilterActionWidget()
{
  qDeleteAll( mActionList );
}

void FilterActionWidget::setAction( const FilterAction* aAction )
{
  bool found = false;
  int count = mComboBox->count() - 1 ; // last entry is the empty one
  QString label = ( aAction ) ? aAction->label() : QString() ;

  // find the index of typeOf(aAction) in mComboBox
  // and clear the other widgets on the way.
  for ( int i = 0; i < count ; i++ ) {
    if ( aAction && mComboBox->itemText(i) == label ) {
      setFilterAction( mActionList.at( i )->createParamWidget( this ) );

      //...set the parameter widget to the settings
      // of aAction...
      aAction->setParamWidgetValue( gl->itemAtPosition( 1, 2 )->widget() );
      //...and show the correct entry of
      // the combo box
      mComboBox->setCurrentIndex(i); // (mm) also raise the widget, but doesn't
      found = true;
    }
  }
  if ( found ) return;

  // not found, so set the empty widget
  setFilterAction();

  mComboBox->setCurrentIndex( count ); // last item
}

FilterAction * FilterActionWidget::action() const
{
  // look up the action description via the label
  // returned by KComboBox::currentText()...
  FilterActionDesc *desc = FilterIf->filterActionDict()->value( mComboBox->currentText() );
  if ( desc ) {
    // ...create an instance...
    FilterAction *fa = desc->create();
    if ( fa ) {
      // ...and apply the setting of the parameter widget.
      fa->applyParamWidgetValue( gl->itemAtPosition( 1, 2 )->widget() );
      return fa;
    }
  }
  return 0;
}


//=============================================================================
//
// class FilterActionWidgetLister (the filter action editor)
//
//=============================================================================

FilterActionWidgetLister:: FilterActionWidgetLister( QWidget *parent, const char* )
  : KWidgetLister( 1, FILTER_MAX_ACTIONS, parent )
{
  mActionList = 0;
}

FilterActionWidgetLister::~FilterActionWidgetLister()
{
}

void FilterActionWidgetLister::setActionList( QList<FilterAction*> *aList )
{
  assert ( aList );

  if ( mActionList )
    regenerateActionListFromWidgets();

  mActionList = aList;

  ((QWidget*)parent())->setEnabled( true );

  if ( aList->count() == 0 ) {
    slotClear();
    return;
  }

  int superfluousItems = (int)mActionList->count() - widgetsMaximum();
  if ( superfluousItems > 0 ) {
    kDebug() << "FilterActionWidgetLister: Clipping action list to"
          << widgetsMaximum() << "items!";

    for ( ; superfluousItems ; superfluousItems-- )
      mActionList->removeLast();
  }

  // set the right number of widgets
  setNumberOfShownWidgetsTo( mActionList->count() );

  // load the actions into the widgets
  QList<QWidget*> widgetList = widgets();
  QList<FilterAction*>::const_iterator aIt;
  QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
  for ( aIt = mActionList->constBegin();
        ( aIt != mActionList->constEnd() && wIt != widgetList.constEnd() );
        ++aIt, ++wIt )
    qobject_cast<FilterActionWidget*>( *wIt )->setAction( ( *aIt ) );
}

void FilterActionWidgetLister::reset()
{
  if ( mActionList )
    regenerateActionListFromWidgets();

  mActionList = 0;
  slotClear();
  ((QWidget*)parent())->setEnabled( false );
}

QWidget* FilterActionWidgetLister::createWidget( QWidget *parent )
{
  return new FilterActionWidget(parent);
}

void FilterActionWidgetLister::clearWidget( QWidget *aWidget )
{
  if ( aWidget )
    ((FilterActionWidget*)aWidget)->setAction(0);
}

void FilterActionWidgetLister::regenerateActionListFromWidgets()
{
  if ( !mActionList ) return;

  mActionList->clear();

  foreach ( const QWidget* w, widgets() ) {
    FilterAction *a = qobject_cast<const FilterActionWidget*>( w )->action();
    if ( a )
      mActionList->append( a );
  }

}

#include "filteractionwidget.moc"
