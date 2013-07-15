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
#include "filteractiondict.h"
#include "filtermanager.h"
#include "mailfilter.h"
#include <pimcommon/widgets/minimumcombobox.h>

#include <KLocalizedString>
#include <KPushButton>

#include <QGridLayout>
#include <QLabel>

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
        : q( qq ), mComboBox( 0 ), mAdd( 0 ), mRemove( 0 ), mLayout( 0 )
    {
    }

    ~Private()
    {
        qDeleteAll( mActionList );
        mActionList.clear();
    }

    void setFilterAction( QWidget *widget = 0 );

    void slotFilterTypeChanged( int index );
    void slotAddWidget();
    void slotRemoveWidget();

    FilterActionWidget *q;
    QList<MailCommon::FilterAction*> mActionList;
    KComboBox *mComboBox;
    KPushButton *mAdd;
    KPushButton *mRemove;

    QGridLayout *mLayout;
};

void FilterActionWidget::Private::setFilterAction( QWidget *widget )
{
    if ( mLayout->itemAtPosition( 1, 2 ) ) {
        delete mLayout->itemAtPosition( 1, 2 )->widget();
    }

    if ( widget ) {
        mLayout->addWidget( widget, 1, 2 );
    } else {
        mLayout->addWidget( new QLabel( i18n( "Please select an action." ), q ), 1, 2 );
    }
}

void FilterActionWidget::Private::slotAddWidget()
{
    emit q->addWidget( q );
    emit q->filterModified();
}

void FilterActionWidget::Private::slotRemoveWidget()
{
    emit q->removeWidget( q );
    emit q->filterModified();
}

void FilterActionWidget::Private::slotFilterTypeChanged( int index )
{
    setFilterAction( index < mActionList.count() ?
                         mActionList.at( index )->createParamWidget( q ) :
                         0 );
}

FilterActionWidget::FilterActionWidget( QWidget *parent )
    : KHBox( parent ), d( new Private( this ) )
{
    QWidget *widget = new QWidget( this );

    d->mLayout = new QGridLayout( widget );
    d->mLayout->setContentsMargins( 0, 0, 0, 0 );

    d->mComboBox = new PimCommon::MinimumComboBox( widget );
    d->mComboBox->setEditable( false );
    Q_ASSERT( d->mComboBox );
    d->mLayout->addWidget( d->mComboBox, 1, 1 );
    d->mAdd = new KPushButton( widget );
    d->mAdd->setIcon( KIcon( "list-add" ) );
    d->mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    d->mRemove = new KPushButton( widget );
    d->mRemove->setIcon( KIcon( "list-remove" ) );
    d->mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    setSpacing( 4 );

    int index;
    QList<FilterActionDesc*> list = MailCommon::FilterManager::filterActionDict()->list();
    QList<FilterActionDesc*>::const_iterator it;
    QList<FilterActionDesc*>::const_iterator end( list.constEnd() );
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {
        //create an instance:
        FilterAction *action = (*it)->create();

        // append to the list of actions:
        d->mActionList.append( action );

        // add (i18n-ized) name to combo box
        d->mComboBox->addItem( (*it)->label,(*it)->name );

        // Register the FilterAction modification signal
        connect( action, SIGNAL(filterActionModified()), this, SIGNAL(filterModified()) );
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
    connect( d->mComboBox, SIGNAL(activated(int)),
             this, SLOT(slotFilterTypeChanged(int)) );

    connect( d->mComboBox, SIGNAL(activated(int)),
             this, SIGNAL(filterModified()) );

    connect( d->mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( d->mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );

    d->setFilterAction();
    d->mLayout->addWidget( d->mAdd, 1, 3 );
    d->mLayout->addWidget( d->mRemove, 1, 4 );

}

FilterActionWidget::~FilterActionWidget()
{
    delete d;
}

void FilterActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    d->mAdd->setEnabled( addButtonEnabled );
    d->mRemove->setEnabled( removeButtonEnabled );
}

void FilterActionWidget::setAction( const FilterAction *action )
{
    bool found = false;
    const int count = d->mComboBox->count() - 1 ; // last entry is the empty one

    const QString name = action ? action->name() : QString();

    // find the index of typeOf(action) in mComboBox
    // and clear the other widgets on the way.
    for ( int i = 0; i < count; ++i ) {
        if ( action && d->mComboBox->itemData( i ) == name ) {
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

    if ( found ) {
        return;
    }

    // not found, so set the empty widget
    d->setFilterAction();

    d->mComboBox->setCurrentIndex( count ); // last item
}

FilterAction *FilterActionWidget::action() const
{
    // look up the action description via the label
    // returned by KComboBox::currentText()...
    FilterActionDesc *description =
            MailCommon::FilterManager::filterActionDict()->value( d->mComboBox->itemData(d->mComboBox->currentIndex()).toString() );

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
    if ( !mActionList ) {
        return;
    }

    mActionList->clear();

    foreach ( const QWidget *widget, q->widgets() ) {
        FilterAction *action = qobject_cast<const FilterActionWidget*>( widget )->action();
        if ( action ) {
            mActionList->append( action );
        }
    }
    q->updateAddRemoveButton();
}

FilterActionWidgetLister:: FilterActionWidgetLister( QWidget *parent )
    : KWidgetLister( false, 1, FILTER_MAX_ACTIONS, parent ), d( new Private( this ) )
{
}

FilterActionWidgetLister::~FilterActionWidgetLister()
{
    delete d;
}

void FilterActionWidgetLister::setActionList( QList<FilterAction*> *list )
{
    Q_ASSERT( list );
    if ( d->mActionList && d->mActionList != list ) {
        d->regenerateActionListFromWidgets();
    }

    d->mActionList = list;

    static_cast<QWidget*>( parent() )->setEnabled( true );

    if ( !widgets().isEmpty() ) { // move this below next 'if'?
        widgets().first()->blockSignals(true);
    }

    if ( list->isEmpty() ) {
        slotClear();
        widgets().first()->blockSignals(false);
        return;
    }

    int superfluousItems = (int)d->mActionList->count() - widgetsMaximum();
    if ( superfluousItems > 0 ) {
        kDebug() << "FilterActionWidgetLister: Clipping action list to"
                 << widgetsMaximum() << "items!";

        for ( ; superfluousItems ; superfluousItems-- ) {
            d->mActionList->removeLast();
        }
    }

    // set the right number of widgets
    setNumberOfShownWidgetsTo( d->mActionList->count() );

    // load the actions into the widgets
    QList<QWidget*> widgetList = widgets();
    QList<FilterAction*>::const_iterator aEnd( d->mActionList->constEnd() );
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( QList<FilterAction*>::const_iterator aIt = d->mActionList->constBegin();
          ( aIt != aEnd && wIt != wEnd ); ++aIt, ++wIt ) {
        FilterActionWidget *w = qobject_cast<FilterActionWidget*>( *wIt );
        w->setAction( ( *aIt ) );
        connect( w, SIGNAL(filterModified()),
                 this, SIGNAL(filterModified()), Qt::UniqueConnection );
        reconnectWidget( w );
    }
    widgets().first()->blockSignals(false);
    updateAddRemoveButton();

}

void FilterActionWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void FilterActionWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}

void FilterActionWidgetLister::updateAddRemoveButton()
{
    QList<QWidget*> widgetList = widgets();
    const int numberOfWidget( widgetList.count() );
    bool addButtonEnabled = false;
    bool removeButtonEnabled = false;
    if ( numberOfWidget <= widgetsMinimum() ) {
        addButtonEnabled = true;
        removeButtonEnabled = false;
    } else if ( numberOfWidget >= widgetsMaximum() ) {
        addButtonEnabled = false;
        removeButtonEnabled = true;
    } else {
        addButtonEnabled = true;
        removeButtonEnabled = true;
    }
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        FilterActionWidget *w = qobject_cast<FilterActionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void FilterActionWidgetLister::updateActionList()
{
    d->regenerateActionListFromWidgets();
}

void FilterActionWidgetLister::reset()
{
    if ( d->mActionList ) {
        d->regenerateActionListFromWidgets();
    }

    d->mActionList = 0;
    slotClear();

    static_cast<QWidget*>( parent() )->setEnabled( false );
}

void FilterActionWidgetLister::reconnectWidget( FilterActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );

    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

QWidget *FilterActionWidgetLister::createWidget( QWidget *parent )
{
    FilterActionWidget *w = new FilterActionWidget( parent );
    reconnectWidget( w );
    return w;
}

void FilterActionWidgetLister::clearWidget( QWidget *widget )
{
    if ( widget ) {
        FilterActionWidget *w = static_cast<FilterActionWidget*>( widget );
        w->setAction( 0 );
        w->disconnect( this );
        reconnectWidget( w ) ;
        updateAddRemoveButton();
    }
}

#include "filteractionwidget.moc"
