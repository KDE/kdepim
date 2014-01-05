/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "numericdoublerulewidgethandler.h"
#include <pimcommon/widgets/minimumcombobox.h>
#include "search/searchpattern.h"
#include "widgets/regexplineedit.h"
using MailCommon::RegExpLineEdit;


#include <KDebug>
#include <KLocale>

#include <KDoubleNumInput>
#include <QStackedWidget>


using namespace MailCommon;

static const struct {
    SearchRule::Function id;
    const char *displayName;
} NumericFunctions[] = {
    { SearchRule::FuncEquals,           I18N_NOOP( "is equal to" )         },
    { SearchRule::FuncNotEqual,         I18N_NOOP( "is not equal to" )      },
    { SearchRule::FuncIsGreater,        I18N_NOOP( "is greater than" )     },
    { SearchRule::FuncIsLessOrEqual,    I18N_NOOP( "is less than or equal to" ) },
    { SearchRule::FuncIsLess,           I18N_NOOP( "is less than" )        },
    { SearchRule::FuncIsGreaterOrEqual, I18N_NOOP( "is greater than or equal to" ) }
};
static const int NumericFunctionCount =
        sizeof( NumericFunctions ) / sizeof( *NumericFunctions );


QWidget *NumericDoubleRuleWidgetHandler::createFunctionWidget(
        int number, QStackedWidget *functionStack, const QObject *receiver, bool /*isNepomukSearch*/ ) const
{
    if ( number != 0 ) {
        return 0;
    }

    PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
    funcCombo->setObjectName( QLatin1String("numericDoubleRuleFuncCombo") );
    for ( int i = 0; i < NumericFunctionCount; ++i ) {
        funcCombo->addItem( i18n( NumericFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    QObject::connect( funcCombo, SIGNAL(activated(int)),
                      receiver, SLOT(slotFunctionChanged()) );
    return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *NumericDoubleRuleWidgetHandler::createValueWidget( int number,
                                                            QStackedWidget *valueStack,
                                                            const QObject *receiver ) const
{
    if ( number != 0 ) {
        return 0;
    }

    KDoubleNumInput *numInput = new KDoubleNumInput( valueStack );
    numInput->setSliderEnabled( false );
    numInput->setObjectName( QLatin1String("KDoubleNumInput") );
    QObject::connect( numInput, SIGNAL(valueChanged(double)),
                      receiver, SLOT(slotValueChanged()) );
    return numInput;
}

//---------------------------------------------------------------------------

SearchRule::Function NumericDoubleRuleWidgetHandler::currentFunction(
        const QStackedWidget *functionStack ) const
{
    const PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericDoubleRuleFuncCombo") );

    if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
        return NumericFunctions[funcCombo->currentIndex()].id;
    }

    return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function NumericDoubleRuleWidgetHandler::function( const QByteArray &field,
                                                               const QStackedWidget *functionStack ) const
{
    if ( !handlesField( field ) ) {
        return SearchRule::FuncNone;
    }

    return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

QString NumericDoubleRuleWidgetHandler::currentValue( const QStackedWidget *valueStack ) const
{
    const KDoubleNumInput *numInput = valueStack->findChild<KDoubleNumInput*>( QLatin1String("KDoubleNumInput") );

    if ( numInput ) {
        return QString::number( int(numInput->value()*1024) );
    }

    return QString();
}

//---------------------------------------------------------------------------

QString NumericDoubleRuleWidgetHandler::value( const QByteArray &field,
                                               const QStackedWidget *,
                                               const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    return currentValue( valueStack );
}

//---------------------------------------------------------------------------

QString NumericDoubleRuleWidgetHandler::prettyValue( const QByteArray &field,
                                                     const QStackedWidget *,
                                                     const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    return currentValue( valueStack );
}

//---------------------------------------------------------------------------

bool NumericDoubleRuleWidgetHandler::handlesField( const QByteArray &field ) const
{
    return field == "<size>";
}

//---------------------------------------------------------------------------

void NumericDoubleRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                            QStackedWidget *valueStack ) const
{
    // reset the function combo box
    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericDoubleRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        funcCombo->setCurrentIndex( 0 );
        funcCombo->blockSignals( false );
    }

    // reset the value widget
    KDoubleNumInput *numInput = valueStack->findChild<KDoubleNumInput*>( QLatin1String("KDoubleNumInput") );

    if ( numInput ) {
        numInput->blockSignals( true );
        numInput->setValue( 0.0 );
        numInput->blockSignals( false );
    }
}

//---------------------------------------------------------------------------

void initDoubleNumInput( KDoubleNumInput *numInput, const QByteArray &field )
{
    if ( field == "<size>" ) {
        numInput->setMinimum( 0 );
        numInput->setSingleStep(1);
        numInput->setSuffix( i18nc( "spinbox suffix: unit for kilobyte", " kB" ) );
        numInput->setSliderEnabled( false );
    }
}

//---------------------------------------------------------------------------

bool NumericDoubleRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                              QStackedWidget *valueStack,
                                              const SearchRule::Ptr rule, bool /*isNepomukSearch*/ ) const
{
    if ( !rule || !handlesField( rule->field() ) ) {
        reset( functionStack, valueStack );
        return false;
    }

    // set the function
    const SearchRule::Function func = rule->function();
    int funcIndex = 0;
    for ( ; funcIndex < NumericFunctionCount; ++funcIndex ) {
        if ( func == NumericFunctions[funcIndex].id ) {
            break;
        }
    }

    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericDoubleRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        if ( funcIndex < NumericFunctionCount ) {
            funcCombo->setCurrentIndex( funcIndex );
        } else {
            funcCombo->setCurrentIndex( 0 );
        }
        funcCombo->blockSignals( false );
        functionStack->setCurrentWidget( funcCombo );
    }

    // set the value
    bool ok;
    int value = rule->contents().toInt( &ok );
    if ( !ok ) {
        value = 0;
    }

    KDoubleNumInput *numInput = valueStack->findChild<KDoubleNumInput*>( QLatin1String("KDoubleNumInput") );

    if ( numInput ) {
        initDoubleNumInput( numInput, rule->field() );
        numInput->blockSignals( true );
        numInput->setValue( value/(1024.0) );
        numInput->blockSignals( false );
        valueStack->setCurrentWidget( numInput );
    }
    return true;
}

//---------------------------------------------------------------------------

bool NumericDoubleRuleWidgetHandler::update( const QByteArray &field,
                                             QStackedWidget *functionStack,
                                             QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return false;
    }

    // raise the correct function widget
    functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("numericDoubleRuleFuncCombo") ) );

    // raise the correct value widget
    KDoubleNumInput *numInput = valueStack->findChild<KDoubleNumInput*>( QLatin1String("KDoubleNumInput") );

    if ( numInput ) {
        initDoubleNumInput( numInput, field );
        valueStack->setCurrentWidget( numInput );
    }
    return true;
}
