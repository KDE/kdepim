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

#include "tagrulewidgethandler.h"
#include "search/searchpattern.h"
#include "widgets/regexplineedit.h"
using MailCommon::RegExpLineEdit;

#include <pimcommon/widgets/minimumcombobox.h>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <Nepomuk2/Tag>


#include <QLineEdit>
#include <QStackedWidget>

using namespace MailCommon;

static const struct {
    SearchRule::Function id;
    const char *displayName;
} TagFunctions[] = {
    { SearchRule::FuncContains,           I18N_NOOP( "contains" )          },
    { SearchRule::FuncContainsNot,        I18N_NOOP( "does not contain" )   },
    { SearchRule::FuncEquals,             I18N_NOOP( "equals" )            },
    { SearchRule::FuncNotEqual,           I18N_NOOP( "does not equal" )     },
    { SearchRule::FuncRegExp,             I18N_NOOP( "matches regular expr." ) },
    { SearchRule::FuncNotRegExp,          I18N_NOOP( "does not match reg. expr." ) }
};
static const int TagFunctionCount =
        sizeof( TagFunctions ) / sizeof( *TagFunctions );

//---------------------------------------------------------------------------

QWidget *TagRuleWidgetHandler::createFunctionWidget(
        int number, QStackedWidget *functionStack, const QObject *receiver, bool isNepomukSearch ) const
{
    if ( number != 0 ) {
        return 0;
    }

    PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
    funcCombo->setObjectName( QLatin1String("tagRuleFuncCombo") );
    for ( int i = 0; i < TagFunctionCount; ++i ) {
        if (isNepomukSearch) {
            if (TagFunctions[i].id == SearchRule::FuncContains || TagFunctions[i].id == SearchRule::FuncContainsNot) {
                funcCombo->addItem( i18n( TagFunctions[i].displayName ) );
            }
        } else {
            funcCombo->addItem( i18n( TagFunctions[i].displayName ) );
        }
    }
    funcCombo->adjustSize();
    QObject::connect( funcCombo, SIGNAL(activated(int)),
                      receiver, SLOT(slotFunctionChanged()) );
    return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *TagRuleWidgetHandler::createValueWidget( int number,
                                                  QStackedWidget *valueStack,
                                                  const QObject *receiver ) const
{
    if ( number == 0 ) {
        RegExpLineEdit *lineEdit = new RegExpLineEdit( valueStack );
        lineEdit->setObjectName( QLatin1String("tagRuleRegExpLineEdit") );
        QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                          receiver, SLOT(slotValueChanged()) );
        QObject::connect( lineEdit, SIGNAL(returnPressed()),
                          receiver, SLOT(slotReturnPressed()) );
        return lineEdit;
    }

    if ( number == 1 ) {
        PimCommon::MinimumComboBox *valueCombo = new PimCommon::MinimumComboBox( valueStack );
        valueCombo->setObjectName( QLatin1String("tagRuleValueCombo") );
        valueCombo->setEditable( true );
        valueCombo->addItem( QString() ); // empty entry for user input
        foreach ( const Nepomuk2::Tag &tag, Nepomuk2::Tag::allTags() ) {
            QString iconName = tag.genericIcon();
            if ( iconName.isEmpty() )
                iconName = QLatin1String("mail-tagged");
            valueCombo->addItem( KIcon( iconName ), tag.label(), tag.uri() );
        }
        valueCombo->adjustSize();
        QObject::connect( valueCombo, SIGNAL(activated(int)),
                          receiver, SLOT(slotValueChanged()) );
        return valueCombo;
    }

    return 0;
}

//---------------------------------------------------------------------------

SearchRule::Function TagRuleWidgetHandler::function( const QByteArray &field,
                                                     const QStackedWidget *functionStack ) const
{
    if ( !handlesField( field ) ) {
        return SearchRule::FuncNone;
    }

    const PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleFuncCombo") );

    if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
        return TagFunctions[funcCombo->currentIndex()].id;
    }
    return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

QString TagRuleWidgetHandler::value( const QByteArray &field,
                                     const QStackedWidget *functionStack,
                                     const QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return QString();
    }

    SearchRule::Function func = function( field, functionStack );
    if ( func == SearchRule::FuncRegExp || func == SearchRule::FuncNotRegExp ) {
        // Use regexp line edit
        const RegExpLineEdit *lineEdit =
                valueStack->findChild<RegExpLineEdit*>( QLatin1String("tagRuleRegExpLineEdit") );

        if ( lineEdit ) {
            return lineEdit->text();
        } else {
            return QString();
        }
    }

    // Use combo box
    const PimCommon::MinimumComboBox *tagCombo =
            valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleValueCombo") );

    if ( tagCombo ) {
        return tagCombo->currentText();
    } else {
        return QString();
    }
}

//---------------------------------------------------------------------------

QString TagRuleWidgetHandler::prettyValue( const QByteArray &field,
                                           const QStackedWidget *funcStack,
                                           const QStackedWidget *valueStack ) const
{
    return value( field, funcStack, valueStack );
}

//---------------------------------------------------------------------------

bool TagRuleWidgetHandler::handlesField( const QByteArray &field ) const
{
    return ( field == "<tag>" );
}

//---------------------------------------------------------------------------

void TagRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                  QStackedWidget *valueStack ) const
{
    // reset the function combo box
    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        funcCombo->setCurrentIndex( 0 );
        funcCombo->blockSignals( false );
    }

    // reset the status value combo box and reg exp line edit
    RegExpLineEdit *lineEdit =
            valueStack->findChild<RegExpLineEdit*>( QLatin1String("tagRuleRegExpLineEdit") );

    if ( lineEdit ) {
        lineEdit->blockSignals( true );
        lineEdit->clear();
        lineEdit->blockSignals( false );
        lineEdit->showEditButton( false );
        valueStack->setCurrentWidget( lineEdit );
    }

    PimCommon::MinimumComboBox *tagCombo = valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleValueCombo") );
    if ( tagCombo ) {
        tagCombo->blockSignals( true );
        tagCombo->setCurrentIndex( 0 );
        tagCombo->blockSignals( false );
    }
}

//---------------------------------------------------------------------------

bool TagRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                    QStackedWidget *valueStack,
                                    const SearchRule::Ptr rule, bool isNepomukSearch ) const
{
    if ( !rule || !handlesField( rule->field() ) ) {
        reset( functionStack, valueStack );
        return false;
    }

    // set the function
    const SearchRule::Function func = rule->function();

    if (isNepomukSearch ) {
        if(func != SearchRule::FuncContains && func != SearchRule::FuncContainsNot) {
            reset( functionStack, valueStack );
            return false;
        }
    }

    int funcIndex = 0;
    for ( ; funcIndex < TagFunctionCount; ++funcIndex ) {
        if ( func == TagFunctions[funcIndex].id ) {
            break;
        }
    }

    PimCommon::MinimumComboBox *funcCombo =
            functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleFuncCombo") );

    if ( funcCombo ) {
        funcCombo->blockSignals( true );
        if ( funcIndex < TagFunctionCount ) {
            funcCombo->setCurrentIndex( funcIndex );
        } else {
            funcCombo->setCurrentIndex( 0 );
        }
        funcCombo->blockSignals( false );
        functionStack->setCurrentWidget( funcCombo );
    }

    // set the value
    if ( func == SearchRule::FuncRegExp || func == SearchRule::FuncNotRegExp ) {
        // set reg exp value
        RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( QLatin1String("tagRuleRegExpLineEdit") );

        if ( lineEdit ) {
            lineEdit->blockSignals( true );
            lineEdit->setText( rule->contents() );
            lineEdit->blockSignals( false );
            lineEdit->showEditButton( true );
            valueStack->setCurrentWidget( lineEdit );
        }
    } else {
        // set combo box value
        int valueIndex = -1;
        int tagIndex = 0;
        foreach ( const Nepomuk2::Tag &tag, Nepomuk2::Tag::allTags() ) {
            if ( tag.label() == rule->contents() ) {
                valueIndex = tagIndex;
                break;
            }
            tagIndex++;
        }

        PimCommon::MinimumComboBox *tagCombo =
                valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleValueCombo") );

        if ( tagCombo ) {
            tagCombo->blockSignals( true );
            if ( valueIndex == -1 ) {
                tagCombo->setCurrentIndex( 0 );
                // Still show tag if it was deleted from MsgTagMgr
                QLineEdit *lineEdit = tagCombo->lineEdit(); // krazy:exclude=qclasses
                Q_ASSERT( lineEdit );
                lineEdit->setText( rule->contents() );
            } else {
                // Existing tags numbered from 1
                tagCombo->setCurrentIndex( valueIndex + 1 );
            }
            tagCombo->blockSignals( false );
            valueStack->setCurrentWidget( tagCombo );
        }
    }
    return true;
}

//---------------------------------------------------------------------------

bool TagRuleWidgetHandler::update( const QByteArray &field,
                                   QStackedWidget *functionStack,
                                   QStackedWidget *valueStack ) const
{
    if ( !handlesField( field ) ) {
        return false;
    }

    // raise the correct function widget
    functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("tagRuleFuncCombo") ) );

    // raise the correct value widget
    SearchRule::Function func = function( field, functionStack );
    if ( func == SearchRule::FuncRegExp || func == SearchRule::FuncNotRegExp ) {
        valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( QLatin1String("tagRuleRegExpLineEdit" )) );
    } else {
        valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( QLatin1String("tagRuleValueCombo") ) );
    }

    return true;
}

