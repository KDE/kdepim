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

#include "messagerulewidgethandler.h"
#include "search/searchpattern.h"
#include "widgets/regexplineedit.h"
using MailCommon::RegExpLineEdit;

#include <pimcommon/widgets/minimumcombobox.h>

#include <QDebug>
#include <KLocale>

#include <QStackedWidget>
#include <QLabel>

using namespace MailCommon;

// also see SearchRule::matches() and SearchRule::Function
// if you change the following strings!
static const struct {
    SearchRule::Function id;
    const char *displayName;
} MessageFunctions[] = {
    { SearchRule::FuncContains,        I18N_NOOP( "contains" )          },
    { SearchRule::FuncContainsNot,     I18N_NOOP( "does not contain" )  },
    { SearchRule::FuncRegExp,          I18N_NOOP( "matches regular expr." ) },
    { SearchRule::FuncNotRegExp,       I18N_NOOP( "does not match reg. expr." ) },
    { SearchRule::FuncHasAttachment,   I18N_NOOP( "has an attachment" ) },
    { SearchRule::FuncHasNoAttachment, I18N_NOOP( "has no attachment" ) },
};
static const int MessageFunctionCount =
        sizeof( MessageFunctions ) / sizeof( *MessageFunctions );

//---------------------------------------------------------------------------

QWidget *MessageRuleWidgetHandler::createFunctionWidget(
        int number, QStackedWidget *functionStack, const QObject *receiver, bool isBalooSearch ) const
{
    if ( number != 0 ) {
        return 0;
    }

    PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
    funcCombo->setObjectName( QLatin1String("messageRuleFuncCombo") );
    for ( int i = 0; i < MessageFunctionCount; ++i ) {
        if ( !( isBalooSearch && (MessageFunctions[i].id == SearchRule::FuncHasAttachment || MessageFunctions[i].id == SearchRule::FuncHasNoAttachment) )) {
            funcCombo->addItem( i18n( MessageFunctions[i].displayName ) );
        }
    }
    funcCombo->adjustSize();
    QObject::connect( funcCombo, SIGNAL(activated(int)),
                      receiver, SLOT(slotFunctionChanged()) );
    return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *MessageRuleWidgetHandler::createValueWidget( int number,
                                                      QStackedWidget *valueStack,
                                                      const QObject *receiver ) const
{
    if ( number == 0 ) {
        RegExpLineEdit *lineEdit =
                new RegExpLineEdit( valueStack );
        lineEdit->setObjectName( QLatin1String("regExpLineEdit") );
        QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                          receiver, SLOT(slotValueChanged()) );
        QObject::connect( lineEdit, SIGNAL(returnPressed()),
                          receiver, SLOT(slotReturnPressed()) );
        return lineEdit;
    }

    // blank QLabel to hide value widget for has-attachment rule
    if ( number == 1 ) {
        QLabel *label = new QLabel( valueStack );
        label->setObjectName( QLatin1String("textRuleValueHider") );
        label->setBuddy( valueStack );
        return label;
    }

    return 0;
}

//---------------------------------------------------------------------------

SearchRule::Function MessageRuleWidgetHandler::currentFunction(
        const QStackedWidget *functionStack ) const
{
    const PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("messageRuleFuncCombo") );

    if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
        return MessageFunctions[funcCombo->currentIndex()].id;
    }

    return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function MessageRuleWidgetHandler::function( const QByteArray & field,
                                                         const QStackedWidget *functionStack ) const
{
    if ( !handlesField( field ) ) {
        return SearchRule::FuncNone;
    }

    return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

QString MessageRuleWidgetHandler::currentValue( const QStackedWidget *valueStack,
                                                SearchRule::Function ) const
{
    const RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

    if ( lineEdit ) {
        return lineEdit->text();
    }

    return QString();
}

//---------------------------------------------------------------------------

QString MessageRuleWidgetHandler::value( const QByteArray &field,
                                         const QStackedWidget *functionStack,
                                         const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    SearchRule::Function func = currentFunction( functionStack );
    if ( func == SearchRule::FuncHasAttachment ) {
        return QLatin1String("has an attachment"); // just a non-empty dummy value
    } else if ( func == SearchRule::FuncHasNoAttachment ) {
        return QLatin1String("has no attachment"); // just a non-empty dummy value
    } else {
        return currentValue( valueStack, func );
    }
}

//---------------------------------------------------------------------------

QString MessageRuleWidgetHandler::prettyValue( const QByteArray & field,
                                               const QStackedWidget *functionStack,
                                               const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    SearchRule::Function func = currentFunction( functionStack );
    if ( func == SearchRule::FuncHasAttachment ) {
        return i18n( "has an attachment" );
    } else if ( func == SearchRule::FuncHasNoAttachment ) {
        return i18n( "has no attachment" );
    } else {
        return currentValue( valueStack, func );
    }
}

//---------------------------------------------------------------------------

bool MessageRuleWidgetHandler::handlesField( const QByteArray & field ) const
{
    return ( field == "<message>" );
}

//---------------------------------------------------------------------------

void MessageRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                      QStackedWidget *valueStack ) const
{
    // reset the function combo box
    PimCommon::MinimumComboBox *funcCombo = functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("messageRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        funcCombo->setCurrentIndex( 0 );
        funcCombo->blockSignals( false );
    }

    // reset the value widget
    RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

    if ( lineEdit ) {
        lineEdit->blockSignals( true );
        lineEdit->clear();
        lineEdit->blockSignals( false );
        lineEdit->showEditButton( false );
        valueStack->setCurrentWidget( lineEdit );
    }
}

//---------------------------------------------------------------------------

bool MessageRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                        QStackedWidget *valueStack,
                                        const SearchRule::Ptr rule, bool isBalooSearch ) const
{
    if ( !rule || !handlesField( rule->field() ) ) {
        reset( functionStack, valueStack );
        return false;
    }

    const SearchRule::Function func = rule->function();

    if ( ( isBalooSearch && (func == SearchRule::FuncHasAttachment || func == SearchRule::FuncHasNoAttachment) )) {
        reset( functionStack, valueStack );
        return false;
    }

    int i = 0;
    for ( ; i < MessageFunctionCount; ++i ) {
        if ( func == MessageFunctions[i].id ) {
            break;
        }
    }

    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("messageRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        if ( i < MessageFunctionCount ) {
            funcCombo->setCurrentIndex( i );
        } else {
            funcCombo->setCurrentIndex( 0 );
        }
        funcCombo->blockSignals( false );
        functionStack->setCurrentWidget( funcCombo );
    }

    if ( func == SearchRule::FuncHasAttachment  ||
         func == SearchRule::FuncHasNoAttachment ) {
        QWidget *w = valueStack->findChild<QWidget*>( QLatin1String("textRuleValueHider") );
        valueStack->setCurrentWidget( w );
    } else {
        RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>(QLatin1String( "regExpLineEdit") );

        if ( lineEdit ) {
            lineEdit->blockSignals( true );
            lineEdit->setText( rule->contents() );
            lineEdit->blockSignals( false );
            lineEdit->showEditButton( func == SearchRule::FuncRegExp ||
                                      func == SearchRule::FuncNotRegExp );
            valueStack->setCurrentWidget( lineEdit );
        }
    }
    return true;
}

//---------------------------------------------------------------------------

bool MessageRuleWidgetHandler::update( const QByteArray & field,
                                       QStackedWidget *functionStack,
                                       QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return false;
    }

    // raise the correct function widget
    functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("messageRuleFuncCombo") ) );

    // raise the correct value widget
    SearchRule::Function func = currentFunction( functionStack );
    if ( func == SearchRule::FuncHasAttachment  ||
         func == SearchRule::FuncHasNoAttachment ) {
        QWidget *w = valueStack->findChild<QWidget*>( QLatin1String("textRuleValueHider") );
        valueStack->setCurrentWidget( w );
    } else {
        RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

        if ( lineEdit ) {
            lineEdit->showEditButton( func == SearchRule::FuncRegExp ||
                                      func == SearchRule::FuncNotRegExp );
            valueStack->setCurrentWidget( lineEdit );
        }
    }
    return true;
}
