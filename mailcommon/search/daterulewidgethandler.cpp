/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "daterulewidgethandler.h"
#include "search/searchpattern.h"

#include <pimcommon/widgets/minimumcombobox.h>

#include <KDebug>
#include <KDateComboBox>
#include <KLocale>

#include <QObject>
#include <QStackedWidget>

using namespace MailCommon;


static const struct {
    SearchRule::Function id;
    const char *displayName;
} DateFunctions[] = {
    { SearchRule::FuncEquals,           I18N_NOOP( "is equal to" )         },
    { SearchRule::FuncNotEqual,         I18N_NOOP( "is not equal to" )      },
    { SearchRule::FuncIsGreater,        I18N_NOOP( "is after" )     },
    { SearchRule::FuncIsLessOrEqual,    I18N_NOOP( "is before or equal to" ) },
    { SearchRule::FuncIsLess,           I18N_NOOP( "is before" )        },
    { SearchRule::FuncIsGreaterOrEqual, I18N_NOOP( "is after or equal to" ) }
};
static const int DateFunctionCount =
        sizeof( DateFunctions ) / sizeof( *DateFunctions );

//---------------------------------------------------------------------------

QWidget *DateRuleWidgetHandler::createFunctionWidget(
        int number, QStackedWidget *functionStack, const QObject *receiver, bool /*isNepomukSearch*/ ) const
{
    if ( number != 0 ) {
        return 0;
    }

    PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
    funcCombo->setObjectName( QLatin1String("dateRuleFuncCombo") );
    for ( int i = 0; i < DateFunctionCount; ++i ) {
        funcCombo->addItem( i18n( DateFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    QObject::connect( funcCombo, SIGNAL(activated(int)),
                      receiver, SLOT(slotFunctionChanged()) );
    return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *DateRuleWidgetHandler::createValueWidget( int number,
                                                   QStackedWidget *valueStack,
                                                   const QObject *receiver ) const
{
    if ( number != 0 ) {
        return 0;
    }

    KDateComboBox *dateCombo = new KDateComboBox( valueStack );
    dateCombo->setObjectName( QLatin1String("KDateComboBox") );
    dateCombo->setOptions( KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords );
    QObject::connect( dateCombo, SIGNAL(dateChanged(QDate)),
                      receiver, SLOT(slotValueChanged()) );
    return dateCombo;
}

//---------------------------------------------------------------------------

SearchRule::Function DateRuleWidgetHandler::currentFunction(
        const QStackedWidget *functionStack ) const
{
    const PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("dateRuleFuncCombo") );

    if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
        return DateFunctions[funcCombo->currentIndex()].id;
    }

    return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function DateRuleWidgetHandler::function( const QByteArray &field,
                                                      const QStackedWidget *functionStack ) const
{
    if ( !handlesField( field ) ) {
        return SearchRule::FuncNone;
    }

    return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

QString DateRuleWidgetHandler::currentValue( const QStackedWidget *valueStack ) const
{
    const KDateComboBox *dateInput = valueStack->findChild<KDateComboBox*>( QLatin1String("KDateComboBox") );

    if ( dateInput ) {
        return dateInput->date().toString( Qt::ISODate );
    }

    return QString();
}

//---------------------------------------------------------------------------

QString DateRuleWidgetHandler::value( const QByteArray &field,
                                      const QStackedWidget *,
                                      const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    return currentValue( valueStack );
}

//---------------------------------------------------------------------------

QString DateRuleWidgetHandler::prettyValue( const QByteArray &field,
                                            const QStackedWidget *,
                                            const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    return currentValue( valueStack );
}

//---------------------------------------------------------------------------

bool DateRuleWidgetHandler::handlesField( const QByteArray &field ) const
{
    return ( field == "<date>" );
}

//---------------------------------------------------------------------------

void DateRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                   QStackedWidget *valueStack ) const
{
    // reset the function combo box
    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("dateRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        funcCombo->setCurrentIndex( 0 );
        funcCombo->blockSignals( false );
    }

    // reset the value widget
    KDateComboBox *dateInput = valueStack->findChild<KDateComboBox*>( QLatin1String("KDateComboBox") );

    if ( dateInput ) {
        dateInput->blockSignals( true );
        dateInput->setDate( QDate::currentDate() );
        dateInput->blockSignals( false );
    }
}

//---------------------------------------------------------------------------

bool DateRuleWidgetHandler::setRule( QStackedWidget *functionStack,
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
    for ( ; funcIndex < DateFunctionCount; ++funcIndex ) {
        if ( func == DateFunctions[funcIndex].id ) {
            break;
        }
    }

    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("dateRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        if ( funcIndex < DateFunctionCount ) {
            funcCombo->setCurrentIndex( funcIndex );
        } else {
            funcCombo->setCurrentIndex( 0 );
        }
        funcCombo->blockSignals( false );
        functionStack->setCurrentWidget( funcCombo );
    }

    // set the value
    const QString value = rule->contents();

    KDateComboBox *dateInput = valueStack->findChild<KDateComboBox*>( QLatin1String("KDateComboBox") );

    if ( dateInput ) {
        dateInput->blockSignals( true );
        dateInput->setDate( QDate::fromString ( value, Qt::ISODate )  );
        dateInput->blockSignals( false );
        valueStack->setCurrentWidget( dateInput );
    }
    return true;
}

//---------------------------------------------------------------------------

bool DateRuleWidgetHandler::update( const QByteArray &field,
                                    QStackedWidget *functionStack,
                                    QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return false;
    }

    // raise the correct function widget
    functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("dateRuleFuncCombo") ) );

    // raise the correct value widget
    KDateComboBox *dateInput = valueStack->findChild<KDateComboBox*>( QLatin1String("KDateComboBox") );

    if ( dateInput ) {
        valueStack->setCurrentWidget( dateInput );
    }
    return true;
}

