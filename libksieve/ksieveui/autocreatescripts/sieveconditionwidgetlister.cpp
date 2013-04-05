/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sieveconditionwidgetlister.h"
#include "pimcommon/minimumcombobox.h"

#include <KPushButton>
#include <KDialog>
#include <KLocale>

#include <QGridLayout>
#include <QLabel>

using namespace KSieveUi;

SieveConditionWidget::SieveConditionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveConditionWidget::~SieveConditionWidget()
{
}

void SieveConditionWidget::setFilterAction( QWidget *widget )
{
    if ( mLayout->itemAtPosition( 1, 2 ) ) {
        delete mLayout->itemAtPosition( 1, 2 )->widget();
    }

    if ( widget ) {
        mLayout->addWidget( widget, 1, 2 );
    } else {
        mLayout->addWidget( new QLabel( i18n( "Please select an condition." ), this ), 1, 2 );
    }
}


void SieveConditionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );
    mComboBox->setMaxCount( mComboBox->count() );
    mComboBox->adjustSize();

    mLayout->addWidget(mComboBox, 1, 1);
    connect( mComboBox, SIGNAL(activated(QString)),
             this, SLOT(slotActionChanged(QString)) );

    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( "list-add" ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( "list-remove" ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mLayout->addWidget( mAdd, 1, 3 );
    mLayout->addWidget( mRemove, 1, 4 );

    // redirect focus to the filter action combo box
    setFocusProxy( mComboBox );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
}

void SieveConditionWidget::slotActionChanged(const QString &action)
{

}


void SieveConditionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveConditionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveConditionWidget::reset()
{

}

void SieveConditionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}


SieveConditionWidgetLister::SieveConditionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, 2, 8, parent)
{
}

SieveConditionWidgetLister::~SieveConditionWidgetLister()
{

}

void SieveConditionWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveConditionWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveConditionWidgetLister::updateAddRemoveButton()
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
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveConditionWidgetLister::reconnectWidget( SieveConditionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveConditionWidgetLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveConditionWidgetLister::createWidget( QWidget *parent )
{
    SieveConditionWidget *w = new SieveConditionWidget( parent);
    reconnectWidget( w );
    return w;
}



void SieveConditionWidgetLister::generatedScript(QString &script)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( *wIt );
        //TODO
    }
}


#include "sieveconditionwidgetlister.moc"
