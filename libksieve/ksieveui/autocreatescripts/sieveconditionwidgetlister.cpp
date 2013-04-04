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

#include <KPushButton>
#include <KDialog>

#include <QHBoxLayout>

using namespace KSieveUi;

SieveConditionWidget::SieveConditionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveConditionWidget::~SieveConditionWidget()
{
}

void SieveConditionWidget::initWidget()
{
    QHBoxLayout *hlay = new QHBoxLayout( this );
    hlay->setSpacing( KDialog::spacingHint() );
    hlay->setMargin( 0 );
/*
    // initialize the header field combo box
    mRuleField = new PimCommon::MinimumComboBox( this );
    mRuleField->setObjectName( "mRuleField" );
    mRuleField->setEditable( true );
    KLineEdit *edit = new KLineEdit;
    edit->setClickMessage( i18n("Choose or type your own criteria"));
    mRuleField->setToolTip(i18n("Choose or type your own criteria"));
    edit->setClearButtonShown(true);
    mRuleField->setLineEdit(edit);
    mRuleField->setTrapReturnKey(true);

    mRuleField->addItems( mFilterFieldList );
    KCompletion *comp = mRuleField->completionObject();
    comp->setIgnoreCase(true);
    comp->insertItems(mFilterFieldList);
    comp->setCompletionMode(KGlobalSettings::CompletionPopupAuto);

    // don't show sliders when popping up this menu
    mRuleField->setMaxCount( mRuleField->count() );
    mRuleField->adjustSize();
    hlay->addWidget( mRuleField );

    // initialize the function/value widget stack
    mFunctionStack = new QStackedWidget( this );
    //Don't expand the widget in vertical direction
    mFunctionStack->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    hlay->addWidget( mFunctionStack );

    mValueStack = new QStackedWidget( this );
    hlay->addWidget( mValueStack );
    hlay->setStretchFactor( mValueStack, 10 );
  */
    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( "list-add" ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    hlay->addWidget( mAdd );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( "list-remove" ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    hlay->addWidget( mRemove );

    //RuleWidgetHandlerManager::instance()->createWidgets( mFunctionStack, mValueStack, this );

    // redirect focus to the header field combo box
    //setFocusProxy( mRuleField );

    /*
    connect( mRuleField, SIGNAL(activated(QString)),
             this, SLOT(slotRuleFieldChanged(QString)) );
    connect( mRuleField, SIGNAL(editTextChanged(QString)),
             this, SLOT(slotRuleFieldChanged(QString)) );
    connect( mRuleField, SIGNAL(editTextChanged(QString)),
             this, SIGNAL(fieldChanged(QString)) );
    */
    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
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

SieveConditionWidgetLister::SieveConditionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, 1, 15, parent)
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
    //TODO
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
    //TODO
}


#include "sieveconditionwidgetlister.moc"
