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

#include "sieveactionwidgetlister.h"
#include "sieveactions/sieveaction.h"
#include "sieveactions/sieveactionlist.h"
#include "pimcommon/minimumcombobox.h"

#include <KPushButton>
#include <KDialog>
#include <KLocale>

#include <QGridLayout>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;

SieveActionWidget::SieveActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveActionWidget::~SieveActionWidget()
{
    qDeleteAll(mActionList);
    mActionList.clear();
}

void SieveActionWidget::setFilterAction( QWidget *widget )
{
    if ( mLayout->itemAtPosition( 1, 2 ) ) {
        delete mLayout->itemAtPosition( 1, 2 )->widget();
    }

    if ( widget ) {
        mLayout->addWidget( widget, 1, 2 );
    } else {
        mLayout->addWidget( new QLabel( i18n( "Please select an action." ), this ), 1, 2 );
    }
}

void SieveActionWidget::generatedScript(QString &script)
{
    qDebug()<<" code :"<<mActionList.at(mComboBox->currentIndex())->code(mLayout->itemAtPosition( 1, 2 )->widget());
    //TODO
}

void SieveActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );
    const QList<KSieveUi::SieveAction*> list = KSieveUi::SieveActionList::actionList();
    QList<KSieveUi::SieveAction*>::const_iterator it;
    QList<KSieveUi::SieveAction*>::const_iterator end( list.constEnd() );
    int index = 0;
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {
        // append to the list of actions:
        mActionList.append( *it );

        // add (i18n-ized) name to combo box
        mComboBox->addItem( (*it)->label(),(*it)->name() );
    }

    mComboBox->setMaxCount( mComboBox->count() );
    mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mComboBox->adjustSize();
    mLayout->addWidget(mComboBox, 1, 1);

    updateGeometry();

    connect( mComboBox, SIGNAL(activated(int)),
             this, SLOT(slotActionChanged(int)) );


    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
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

void SieveActionWidget::slotActionChanged(int index)
{
    setFilterAction( index < mActionList.count() ?
                         mActionList.at( index )->createParamWidget( this ) :
                         0 );
}

void SieveActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

SieveActionWidgetLister::SieveActionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, 1, 8, parent)
{
    slotClear();
    updateAddRemoveButton();
}

SieveActionWidgetLister::~SieveActionWidgetLister()
{

}

void SieveActionWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveActionWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveActionWidgetLister::updateAddRemoveButton()
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
        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveActionWidgetLister::generatedScript(QString &script)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( *wIt );
        w->generatedScript(script);
    }
}

void SieveActionWidgetLister::reconnectWidget( SieveActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveActionWidgetLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveActionWidgetLister::createWidget( QWidget *parent )
{
    SieveActionWidget *w = new SieveActionWidget( parent);
    reconnectWidget( w );
    return w;
}


#include "sieveactionwidgetlister.moc"
