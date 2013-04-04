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
#include "pimcommon/minimumcombobox.h"


#include <KPushButton>
#include <KDialog>

#include <QHBoxLayout>

using namespace KSieveUi;

SieveActionWidget::SieveActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveActionWidget::~SieveActionWidget()
{
}

void SieveActionWidget::initWidget()
{
    QHBoxLayout *hlay = new QHBoxLayout( this );
    hlay->setSpacing( KDialog::spacingHint() );
    hlay->setMargin( 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );
    hlay->addWidget(mComboBox);
    connect( mComboBox, SIGNAL(activated(QString)),
             this, SLOT(slotActionChanged(QString)) );

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

void SieveActionWidget::slotActionChanged(const QString &action)
{

}

void SieveActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}


SieveActionWidgetLister::SieveActionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, 1, 15, parent)
{
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

}

void SieveActionWidgetLister::generatedScript(QString &script)
{
    //TODO
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
