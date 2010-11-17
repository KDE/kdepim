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
#include "mailfilter.h"
#include "mailkernel.h"
#include "minimumcombobox.h"

#include <KLocalizedString>

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

using namespace MailCommon;

//=============================================================================
//
// class FilterActionWidget
//
//=============================================================================

class FilterActionWidget::Private
{
  public:
    Private( FilterActionWidget *qq )
      : q( qq )
    {
    }

    ~Private()
    {
      qDeleteAll( mActionList );
    }

    void setFilterAction( QWidget *widget = 0 );

    void slotFilterTypeChanged( int index );

    FilterActionWidget *q;
    QList<MailCommon::FilterAction*> mActionList;
    KComboBox *mComboBox;

    QGridLayout *mLayout;
};

void FilterActionWidget::Private::setFilterAction( QWidget *widget )
{
  if ( mLayout->itemAtPosition( 1, 2 ) )
    delete mLayout->itemAtPosition( 1, 2 )->widget();

  if ( widget )
    mLayout->addWidget( widget, 1, 2 );
  else
    mLayout->addWidget( new QLabel( i18n( "Please select an action." ), q ), 1, 2 );
}

void FilterActionWidget::Private::slotFilterTypeChanged( int index )
{
  setFilterAction( index < mActionList.count() ? mActionList.at( index )->createParamWidget( q ) : 0 );
}


FilterActionWidget::FilterActionWidget( QWidget *parent )
  : KHBox( parent ), d( new Private( this ) )
{
  QWidget *widget = new QWidget( this );

  d->mLayout = new QGridLayout( widget );
  d->mLayout->setContentsMargins( 0, 0, 0, 0 );

  d->mComboBox = new MinimumComboBox( widget );
  d->mComboBox->setEditable( false );
  Q_ASSERT( d->mComboBox );
  d->mLayout->addWidget( d->mComboBox, 1, 1 );

  setSpacing( 4 );

  int index;
  QList<FilterActionDesc*> list = FilterIf->filterActionDict()->list();
  QList<FilterActionDesc*>::const_iterator it;
  for ( index = 0, it = list.constBegin() ; it != list.constEnd() ; ++it, ++index ) {
    //create an instance:
    FilterAction *action = (*it)->create();

    // append to the list of actions:
    d->mActionList.append( action );

    // add (i18n-ized) name to combo box
    d->mComboBox->addItem( (*it)->label );
  }

  // widget for the case where no action is selected.
  d->mComboBox->addItem( " " );
  d->mComboBox->setCurrentIndex( index );

  // don't show scroll bars.
  d->mComboBox->setMaxCount( d->mComboBox->count() );

  // layout management:
  // o the combo box is not to be made larger than it's sizeHint(),
  //   the parameter widget should grow instead.
  // o the whole widget takes all space horizontally, but is fixed vertically.
  d->mComboBox->adjustSize();
  d->mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  updateGeometry();

  // redirect focus to the filter action combo box
  setFocusProxy( d->mComboBox );

  // now connect the combo box and the widget stack
  connect( d->mComboBox, SIGNAL( activated( int ) ),
           this, SLOT( slotFilterTypeChanged( int ) ) );

  d->setFilterAction();
}

FilterActionWidget::~FilterActionWidget()
{
  delete d;
}

void FilterActionWidget::setAction( const FilterAction *action )
{
  bool found = false;
  const int count = d->mComboBox->count() - 1 ; // last entry is the empty one
  const QString label = (action ? action->label() : QString());

  // find the index of typeOf(action) in mComboBox
  // and clear the other widgets on the way.
  for ( int i = 0; i < count ; i++ ) {
    if ( action && d->mComboBox->itemText( i ) == label ) {
      d->setFilterAction( d->mActionList.at( i )->createParamWidget( this ) );

      //...set the parameter widget to the settings
      // of aAction...
      action->setParamWidgetValue( d->mLayout->itemAtPosition( 1, 2 )->widget() );

      //...and show the correct entry of
      // the combo box
      d->mComboBox->setCurrentIndex( i ); // (mm) also raise the widget, but doesn't
      found = true;
    }
  }

  if ( found )
    return;

  // not found, so set the empty widget
  d->setFilterAction();

  d->mComboBox->setCurrentIndex( count ); // last item
}

FilterAction* FilterActionWidget::action() const
{
  // look up the action description via the label
  // returned by KComboBox::currentText()...
  FilterActionDesc *description = FilterIf->filterActionDict()->value( d->mComboBox->currentText() );
  if ( description ) {
    // ...create an instance...
    FilterAction *action = description->create();
    if ( action ) {
      // ...and apply the setting of the parameter widget.
      action->applyParamWidgetValue( d->mLayout->itemAtPosition( 1, 2 )->widget() );
      return action;
    }
  }

  return 0;
}


//=============================================================================
//
// class FilterActionWidgetLister (the filter action editor)
//
//=============================================================================

class FilterActionWidgetLister::Private
{
  public:
    Private( FilterActionWidgetLister *qq )
      : q( qq ), mActionList( 0 )
    {
    }

    void regenerateActionListFromWidgets();

    FilterActionWidgetLister *q;
    QList<MailCommon::FilterAction*> *mActionList;
};

void FilterActionWidgetLister::Private::regenerateActionListFromWidgets()
{
  if ( !mActionList )
    return;

  mActionList->clear();

  foreach ( const QWidget *widget, q->widgets() ) {
    FilterAction *action = qobject_cast<const FilterActionWidget*>( widget )->action();
    if ( action )
      mActionList->append( action );
  }
}


FilterActionWidgetLister:: FilterActionWidgetLister( QWidget *parent )
  : KWidgetLister( 1, FILTER_MAX_ACTIONS, parent ), d( new Private( this ) )
{
}

FilterActionWidgetLister::~FilterActionWidgetLister()
{
  delete d;
}

void FilterActionWidgetLister::setActionList( QList<FilterAction*> *list )
{
  Q_ASSERT( list );

  if ( d->mActionList )
    d->regenerateActionListFromWidgets();

  d->mActionList = list;

  static_cast<QWidget*>( parent() )->setEnabled( true );

  if ( list->count() == 0 ) {
    slotClear();
    return;
  }

  int superfluousItems = (int)d->mActionList->count() - widgetsMaximum();
  if ( superfluousItems > 0 ) {
    kDebug() << "FilterActionWidgetLister: Clipping action list to"
             << widgetsMaximum() << "items!";

    for ( ; superfluousItems ; superfluousItems-- )
      d->mActionList->removeLast();
  }

  // set the right number of widgets
  setNumberOfShownWidgetsTo( d->mActionList->count() );

  // load the actions into the widgets
  QList<QWidget*> widgetList = widgets();
  QList<FilterAction*>::const_iterator aIt;
  QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
  for ( aIt = d->mActionList->constBegin();
        ( aIt != d->mActionList->constEnd() && wIt != widgetList.constEnd() );
        ++aIt, ++wIt )
    qobject_cast<FilterActionWidget*>( *wIt )->setAction( ( *aIt ) );
}

void FilterActionWidgetLister::updateActionList()
{
  d->regenerateActionListFromWidgets();
}

void FilterActionWidgetLister::reset()
{
  if ( d->mActionList )
    d->regenerateActionListFromWidgets();

  d->mActionList = 0;
  slotClear();

  static_cast<QWidget*>( parent() )->setEnabled( false );
}

QWidget* FilterActionWidgetLister::createWidget( QWidget *parent )
{
  return new FilterActionWidget( parent );
}

void FilterActionWidgetLister::clearWidget( QWidget *widget )
{
  if ( widget )
    static_cast<FilterActionWidget*>( widget )->setAction( 0 );
}

#include "filteractionwidget.moc"
