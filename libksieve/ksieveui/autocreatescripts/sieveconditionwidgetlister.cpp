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
#include "sieveconditions/sieveconditionlist.h"
#include "sieveconditions/sievecondition.h"
#include "pimcommon/minimumcombobox.h"

#include <KPushButton>
#include <KDialog>
#include <KLocale>

#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QDebug>

using namespace KSieveUi;

static int MINIMUMCONDITION = 1;
static int MAXIMUMCONDITION = 8;

SieveConditionWidget::SieveConditionWidget(const QStringList &capabilities, QWidget *parent)
    : QWidget(parent)
{
    initWidget(capabilities);
}

SieveConditionWidget::~SieveConditionWidget()
{
}

void SieveConditionWidget::setFilterCondition( QWidget *widget )
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

void SieveConditionWidget::generatedScript(QString &script, QStringList &requires)
{
    const int index = mComboBox->currentIndex();
    if (index != mComboBox->count()-1) {
        KSieveUi::SieveCondition *widgetCondition = mConditionList.at(mComboBox->currentIndex());
        const QStringList lstRequires = widgetCondition->needRequires();
        Q_FOREACH (const QString &r, lstRequires) {
            if (!requires.contains(r)) {
                requires.append(r);
            }
        }
        script += mConditionList.at(mComboBox->currentIndex())->code(mLayout->itemAtPosition( 1, 2 )->widget()) + QLatin1Char('\n');
    }
}

void SieveConditionWidget::initWidget(const QStringList &capabilities)
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );

    const QList<KSieveUi::SieveCondition*> list = KSieveUi::SieveConditionList::conditionList();
    QList<KSieveUi::SieveCondition*>::const_iterator it;
    QList<KSieveUi::SieveCondition*>::const_iterator end( list.constEnd() );
    int index = 0;
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {        
        if ((*it)->needCheckIfServerHasCapability()) {
            if (capabilities.contains((*it)->serverNeedsCapability())) {
                // append to the list of actions:
                mConditionList.append( *it );

                // add (i18n-ized) name to combo box
                mComboBox->addItem( (*it)->label(),(*it)->name() );
            }
        } else {
            // append to the list of actions:
            mConditionList.append( *it );

            // add (i18n-ized) name to combo box
            mComboBox->addItem( (*it)->label(),(*it)->name() );
        }
    }
    mComboBox->addItem(QLatin1String(""));
    mComboBox->setCurrentIndex(mComboBox->count()-1);
    mLayout->addWidget(mComboBox, 1, 1);
    connect( mComboBox, SIGNAL(activated(int)),
             this, SLOT(slotConditionChanged(int)) );

    mComboBox->setMaxCount( mComboBox->count() );
    mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mComboBox->adjustSize();

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
    setFilterCondition(0);
}

void SieveConditionWidget::slotConditionChanged(int index)
{
    setFilterCondition( index < mConditionList.count() ?
                         mConditionList.at( index )->createParamWidget( this ) :
                         0 );
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
    //TODO
}

void SieveConditionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

SieveConditionWidgetLister::SieveConditionWidgetLister(const QStringList &capabilities, QWidget *parent)
    : KPIM::KWidgetLister(false, MINIMUMCONDITION, MAXIMUMCONDITION, parent),
      mSieveCapabilities(capabilities)
{
    slotClear();
    updateAddRemoveButton();
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
    SieveConditionWidget *w = new SieveConditionWidget( mSieveCapabilities, parent);
    reconnectWidget( w );
    return w;
}

void SieveConditionWidgetLister::generatedScript(QString &script, int &numberOfCondition, QStringList &requires)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    bool wasFirst = true;
    for ( ; wIt != wEnd ;++wIt ) {
        QString condition;
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( *wIt );
        w->generatedScript(condition, requires);
        if (!condition.isEmpty()) {
            if (!wasFirst) {
                script += QLatin1String(", ");
            }
            script += condition;
            wasFirst = false;
            ++numberOfCondition;
        }
    }
}

int SieveConditionWidgetLister::conditionNumber() const
{
    return widgets().count();
}

#include "sieveconditionwidgetlister.moc"
