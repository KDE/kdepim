/*  -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2004 Ingo Kloecker <kloecker@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "rulewidgethandlermanager.h"
#include "interfaces/rulewidgethandler.h"

#include <messageviewer/viewer/stl_util.h>
#include <pimcommon/widgets/minimumcombobox.h>

#include <Nepomuk2/Tag>

#include <KDebug>
#include <KIconLoader>
#include <KIcon>

#include <QLineEdit>
#include <QObject>
#include <QStackedWidget>

#include <algorithm>
using std::for_each;
using std::remove;

using namespace MailCommon;

MailCommon::RuleWidgetHandlerManager *MailCommon::RuleWidgetHandlerManager::self = 0;

namespace {

class TextRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    TextRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~TextRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray &field,
                                   const QStackedWidget *functionStack ) const;

    QString value( const QByteArray &field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray &field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray &field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack,
                          SearchRule::Function func ) const;
};

class MessageRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    MessageRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~MessageRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray & field,
                                   const QStackedWidget *functionStack ) const;

    QString value( const QByteArray &field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray & field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray & field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack,
                          SearchRule::Function func ) const;
};

class StatusRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    StatusRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~StatusRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray &field,
                                   const QStackedWidget *functionStack ) const;

    QString value( const QByteArray &field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray &field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray &field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    int currentStatusValue( const QStackedWidget *valueStack ) const;
};


class TagRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    TagRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~TagRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray &field,
                                   const QStackedWidget *functionStack ) const;

    QString value( const QByteArray &field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray &field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray &field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;
};


class NumericRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    NumericRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~NumericRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray & field,
                                     const QStackedWidget *functionStack ) const;

    QString value( const QByteArray & field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray & field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray & field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack ) const;
};

class DateRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    DateRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~DateRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray & field,
                                     const QStackedWidget *functionStack ) const;

    QString value( const QByteArray & field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray & field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray & field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack ) const;
};

class NumericDoubleRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
  public:
    NumericDoubleRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~NumericDoubleRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget( int number,
                                   QStackedWidget *functionStack,
                                   const QObject *receiver ) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray & field,
                                     const QStackedWidget *functionStack ) const;

    QString value( const QByteArray & field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray & field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray & field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

  private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack ) const;
};


}

MailCommon::RuleWidgetHandlerManager::RuleWidgetHandlerManager()
{
  registerHandler( new TagRuleWidgetHandler() );
  registerHandler( new DateRuleWidgetHandler() );
  registerHandler( new NumericRuleWidgetHandler() );
  registerHandler( new StatusRuleWidgetHandler() );
  registerHandler( new MessageRuleWidgetHandler() );
  registerHandler( new NumericDoubleRuleWidgetHandler() );
   // the TextRuleWidgetHandler is the fallback handler, so it has to be added
  // as last handler
  registerHandler( new TextRuleWidgetHandler() );
}

MailCommon::RuleWidgetHandlerManager::~RuleWidgetHandlerManager()
{
  for_each( mHandlers.begin(), mHandlers.end(),
            MessageViewer::DeleteAndSetToZero<RuleWidgetHandler>() );
}

void MailCommon::RuleWidgetHandlerManager::registerHandler( const RuleWidgetHandler *handler )
{
  if ( !handler ) {
    return;
  }
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void MailCommon::RuleWidgetHandlerManager::unregisterHandler( const RuleWidgetHandler *handler )
{
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

namespace {

/**
 * Returns the number of immediate children of parent with the given object name.
 * Used by RuleWidgetHandlerManager::createWidgets().
 */
int childCount( const QObject *parent, const QString &objName )
{
  QObjectList list = parent->children();
  QObject *item;
  int count = 0;
  foreach ( item, list ) {
    if ( item->objectName() == objName ) {
      count++;
    }
  }
  return count;
}

}

void MailCommon::RuleWidgetHandlerManager::createWidgets( QStackedWidget *functionStack,
                                                          QStackedWidget *valueStack,
                                                          const QObject *receiver ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    QWidget *w = 0;
    for ( int i = 0;
          ( w = (*it)->createFunctionWidget( i, functionStack, receiver ) );
          ++i ) {
      if ( childCount( functionStack, w->objectName() ) < 2 ) {
        // there wasn't already a widget with this name, so add this widget
        functionStack->addWidget( w );
      } else {
        // there was already a widget with this name, so discard this widget
        delete w;
        w = 0;
      }
    }
    for ( int i = 0;
          ( w = (*it)->createValueWidget( i, valueStack, receiver ) );
          ++i ) {
      if ( childCount( valueStack, w->objectName() ) < 2 ) {
        // there wasn't already a widget with this name, so add this widget
        valueStack->addWidget( w );
      } else {
        // there was already a widget with this name, so discard this widget
        delete w;
        w = 0;
      }
    }
  }
}

SearchRule::Function MailCommon::RuleWidgetHandlerManager::function(
  const QByteArray &field, const QStackedWidget *functionStack ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    const SearchRule::Function func = (*it)->function( field, functionStack );
    if ( func != SearchRule::FuncNone ) {
      return func;
    }
  }
  return SearchRule::FuncNone;
}

QString MailCommon::RuleWidgetHandlerManager::value( const QByteArray &field,
                                                     const QStackedWidget *functionStack,
                                                     const QStackedWidget *valueStack ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    const QString val = (*it)->value( field, functionStack, valueStack );
    if ( !val.isEmpty() ) {
      return val;
    }
  }
  return QString();
}

QString MailCommon::RuleWidgetHandlerManager::prettyValue( const QByteArray &field,
                                                           const QStackedWidget *functionStack,
                                                           const QStackedWidget *valueStack ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    const QString val = (*it)->prettyValue( field, functionStack, valueStack );
    if ( !val.isEmpty() ) {
      return val;
    }
  }
  return QString();
}

void MailCommon::RuleWidgetHandlerManager::reset( QStackedWidget *functionStack,
                                                  QStackedWidget *valueStack ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    (*it)->reset( functionStack, valueStack );
  }
  update( "", functionStack, valueStack );
}

void MailCommon::RuleWidgetHandlerManager::setRule( QStackedWidget *functionStack,
                                                    QStackedWidget *valueStack,
                                                    const SearchRule::Ptr rule ) const
{
  Q_ASSERT( rule );
  reset( functionStack, valueStack );
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    if ( (*it)->setRule( functionStack, valueStack, rule ) ) {
      return;
    }
  }
}

void MailCommon::RuleWidgetHandlerManager::update( const QByteArray &field,
                                                   QStackedWidget *functionStack,
                                                   QStackedWidget *valueStack ) const
{
  const_iterator end( mHandlers.constEnd() );
  for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
    if ( (*it)->update( field, functionStack, valueStack ) ) {
      return;
    }
  }
}

//-----------------------------------------------------------------------------

// these includes are temporary and should not be needed for the code
// above this line, so they appear only here:
#include "search/searchpattern.h"
#include "widgets/regexplineedit.h"
using MailCommon::RegExpLineEdit;

#include <KLocale>
#include <KNumInput>
#include <KDateComboBox>

#include <QLabel>

//=============================================================================
//
// class TextRuleWidgetHandler
//
//=============================================================================

namespace {

// also see SearchRule::matches() and SearchRule::Function
// if you change the following strings!
static const struct {
  SearchRule::Function id;
  const char *displayName;
} TextFunctions[] = {
  { SearchRule::FuncContains,           I18N_NOOP( "contains" )          },
  { SearchRule::FuncContainsNot,        I18N_NOOP( "does not contain" )   },
  { SearchRule::FuncEquals,             I18N_NOOP( "equals" )            },
  { SearchRule::FuncNotEqual,           I18N_NOOP( "does not equal" )     },
  { SearchRule::FuncStartWith,          I18N_NOOP( "starts with" )         },
  { SearchRule::FuncNotStartWith,       I18N_NOOP( "does not start with" )},
  { SearchRule::FuncEndWith,            I18N_NOOP( "ends with" )           },
  { SearchRule::FuncNotEndWith,         I18N_NOOP( "does not end with" )  },

  { SearchRule::FuncRegExp,             I18N_NOOP( "matches regular expr." ) },
  { SearchRule::FuncNotRegExp,          I18N_NOOP( "does not match reg. expr." ) },
  { SearchRule::FuncIsInAddressbook,    I18N_NOOP( "is in address book" ) },
  { SearchRule::FuncIsNotInAddressbook, I18N_NOOP( "is not in address book" ) }
#if 0
  ,
  { SearchRule::FuncIsInCategory,       I18N_NOOP( "is in category" ) },
  { SearchRule::FuncIsNotInCategory,    I18N_NOOP( "is not in category" ) }
#endif
};
static const int TextFunctionCount =
  sizeof( TextFunctions ) / sizeof( *TextFunctions );

//---------------------------------------------------------------------------

QWidget *TextRuleWidgetHandler::createFunctionWidget(
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
  funcCombo->setObjectName( QLatin1String("textRuleFuncCombo") );
  for ( int i = 0; i < TextFunctionCount; ++i ) {
    funcCombo->addItem( i18n( TextFunctions[i].displayName ) );
  }
  funcCombo->adjustSize();
  QObject::connect( funcCombo, SIGNAL(activated(int)),
                    receiver, SLOT(slotFunctionChanged()) );
  return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *TextRuleWidgetHandler::createValueWidget( int number,
                                                   QStackedWidget *valueStack,
                                                   const QObject *receiver ) const
{
  if ( number == 0 ) {
    RegExpLineEdit *lineEdit = new RegExpLineEdit( valueStack );
    lineEdit->setObjectName( QLatin1String("regExpLineEdit") );
    QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                      receiver, SLOT(slotValueChanged()) );
    QObject::connect( lineEdit, SIGNAL(returnPressed()),
                      receiver, SLOT(slotReturnPressed()) );
    return lineEdit;
  }

  // blank QLabel to hide value widget for in-address-book rule
  if ( number == 1 ) {
    QLabel *label = new QLabel( valueStack );
    label->setObjectName( QLatin1String("textRuleValueHider") );
    label->setBuddy( valueStack );
    return label;
  }
#if 0

//FIXME: review what is this about, why is nepomuk used

  if ( number == 2 ) {
    PimCommon::MinimumComboBox *combo =  new PimCommon::MinimumComboBox( valueStack );
    combo->setObjectName( QLatin1String("categoryCombo") );
    foreach ( const Nepomuk2::Tag &tag, Nepomuk2::Tag::allTags() ) {
      if ( tag.genericIcon().isEmpty() ) {
        combo->addItem( tag.label(), tag.uri() );
      } else {
        combo->addItem( KIcon( tag.genericIcon() ), tag.label(), tag.uri() );
      }
    }
    QObject::connect( combo, SIGNAL(activated(int)),
                      receiver, SLOT(slotValueChanged()) );
    return combo;
  }
#endif
  return 0;
}

//---------------------------------------------------------------------------

SearchRule::Function TextRuleWidgetHandler::currentFunction(
  const QStackedWidget *functionStack ) const
{
  const PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String( "textRuleFuncCombo" ) );

  if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
    return TextFunctions[funcCombo->currentIndex()].id;
  }

  return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function TextRuleWidgetHandler::function( const QByteArray &,
                                                      const QStackedWidget *functionStack ) const
{
  return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

QString TextRuleWidgetHandler::currentValue( const QStackedWidget *valueStack,
                                             SearchRule::Function func ) const
{
#if 0
  // here we gotta check the combobox which contains the categories
  if ( func  == SearchRule::FuncIsInCategory ||
       func  == SearchRule::FuncIsNotInCategory ) {
    const PimCommon::MinimumComboBox *combo = valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("categoryCombo") );

    if ( combo ) {
      return combo->currentText();
    } else {
      return QString();
    }
  }
#endif

  //in other cases of func it is a lineedit
  const RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

  if ( lineEdit ) {
    return lineEdit->text();
  }

  // or anything else, like addressbook
  return QString();
}

//---------------------------------------------------------------------------

QString TextRuleWidgetHandler::value( const QByteArray &,
                                      const QStackedWidget *functionStack,
                                      const QStackedWidget *valueStack ) const
{
  SearchRule::Function func = currentFunction( functionStack );
  if ( func == SearchRule::FuncIsInAddressbook ) {
    return "is in address book"; // just a non-empty dummy value
  } else if ( func == SearchRule::FuncIsNotInAddressbook ) {
    return "is not in address book"; // just a non-empty dummy value
  } else {
    return currentValue( valueStack, func );
  }
}

//---------------------------------------------------------------------------

QString TextRuleWidgetHandler::prettyValue( const QByteArray &,
                                            const QStackedWidget *functionStack,
                                            const QStackedWidget *valueStack ) const
{
  SearchRule::Function func = currentFunction( functionStack );

  if ( func == SearchRule::FuncIsInAddressbook ) {
    return i18n( "is in address book" );
  } else if ( func == SearchRule::FuncIsNotInAddressbook ) {
    return i18n( "is not in address book" );
  } else {
    return currentValue( valueStack, func );
  }
}

//---------------------------------------------------------------------------

bool TextRuleWidgetHandler::handlesField( const QByteArray & ) const
{
  return true; // we handle all fields (as fallback)
}

//---------------------------------------------------------------------------

void TextRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                   QStackedWidget *valueStack ) const
{
  // reset the function combo box
  PimCommon::MinimumComboBox *funcCombo = functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("textRuleFuncCombo") );

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

  PimCommon::MinimumComboBox *combo = valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("categoryCombo") );

  if ( combo ) {
    combo->blockSignals( true );
    combo->setCurrentIndex( 0 );
    combo->blockSignals( false );
  }
#endif
}

//---------------------------------------------------------------------------

bool TextRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                     QStackedWidget *valueStack,
                                     const SearchRule::Ptr rule ) const
{
  if ( !rule ) {
    reset( functionStack, valueStack );
    return false;
  }

  const SearchRule::Function func = rule->function();
  int i = 0;
  for ( ; i < TextFunctionCount; ++i ) {
    if ( func == TextFunctions[i].id ) {
      break;
    }
  }

  PimCommon::MinimumComboBox *funcCombo = functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("textRuleFuncCombo") );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    if ( i < TextFunctionCount ) {
      funcCombo->setCurrentIndex( i );
    } else {
      funcCombo->setCurrentIndex( 0 );
    }
    funcCombo->blockSignals( false );
    functionStack->setCurrentWidget( funcCombo );
  }

  if ( func == SearchRule::FuncIsInAddressbook ||
       func == SearchRule::FuncIsNotInAddressbook ) {
    QWidget *w = valueStack->findChild<QWidget*>( QLatin1String("textRuleValueHider") );
    valueStack->setCurrentWidget( w );
  }
#if 0
  else if ( func == SearchRule::FuncIsInCategory ||
            func == SearchRule::FuncIsNotInCategory ) {
    PimCommon::MinimumComboBox *combo = valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("categoryCombo") );

    combo->blockSignals( true );
    const int numberOfElement( combo->count() );
    for ( i = 0; i < numberOfElement; ++i ) {
      if ( rule->contents() == combo->itemText( i ) ) {
        combo->setCurrentIndex( i );
        break;
      }
    }

    if ( i == combo->count() ) {
      combo->setCurrentIndex( 0 );
    }
    combo->blockSignals( false );
    valueStack->setCurrentWidget( combo );
  }
#endif
  else {
    RegExpLineEdit *lineEdit =
      valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

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

bool TextRuleWidgetHandler::update( const QByteArray &,
                                    QStackedWidget *functionStack,
                                    QStackedWidget *valueStack ) const
{
  // raise the correct function widget
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("textRuleFuncCombo") ) );

  // raise the correct value widget
  SearchRule::Function func = currentFunction( functionStack );
  if ( func == SearchRule::FuncIsInAddressbook ||
       func == SearchRule::FuncIsNotInAddressbook ) {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( QLatin1String("textRuleValueHider") ) );
  }
#if 0
  else if ( func == SearchRule::FuncIsInCategory ||
            func == SearchRule::FuncIsNotInCategory ) {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( QLatin1String("categoryCombo") ) );
  }
#endif
  else {
    RegExpLineEdit *lineEdit =
      valueStack->findChild<RegExpLineEdit*>( QLatin1String("regExpLineEdit") );

    if ( lineEdit ) {
      lineEdit->showEditButton( func == SearchRule::FuncRegExp ||
                                func == SearchRule::FuncNotRegExp );
      valueStack->setCurrentWidget( lineEdit );
    }
  }
  return true;
}

} // anonymous namespace for TextRuleWidgetHandler

//=============================================================================
//
// class MessageRuleWidgetHandler
//
//=============================================================================

namespace {

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
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
  funcCombo->setObjectName( QLatin1String("messageRuleFuncCombo") );
  for ( int i = 0; i < MessageFunctionCount; ++i ) {
    funcCombo->addItem( i18n( MessageFunctions[i].displayName ) );
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
                                        const SearchRule::Ptr rule ) const
{
  if ( !rule || !handlesField( rule->field() ) ) {
    reset( functionStack, valueStack );
    return false;
  }

  const SearchRule::Function func = rule->function();
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

} // anonymous namespace for MessageRuleWidgetHandler

//=============================================================================
//
// class StatusRuleWidgetHandler
//
//=============================================================================

namespace {

static const struct {
  SearchRule::Function id;
  const char *displayName;
} StatusFunctions[] = {
  { SearchRule::FuncContains,    I18N_NOOP( "is" )    },
  { SearchRule::FuncContainsNot, I18N_NOOP( "is not" ) }
};
static const int StatusFunctionCount =
  sizeof( StatusFunctions ) / sizeof( *StatusFunctions );

//---------------------------------------------------------------------------

QWidget *StatusRuleWidgetHandler::createFunctionWidget(
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
  funcCombo->setObjectName( QLatin1String("statusRuleFuncCombo") );
  for ( int i = 0; i < StatusFunctionCount; ++i ) {
    funcCombo->addItem( i18n( StatusFunctions[i].displayName ) );
  }
  funcCombo->adjustSize();
  QObject::connect( funcCombo, SIGNAL(activated(int)),
                    receiver, SLOT(slotFunctionChanged()) );
  return funcCombo;
}

//---------------------------------------------------------------------------

QWidget * StatusRuleWidgetHandler::createValueWidget( int number,
                                                      QStackedWidget *valueStack,
                                                      const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *statusCombo = new PimCommon::MinimumComboBox( valueStack );
  statusCombo->setObjectName( QLatin1String("statusRuleValueCombo") );
  for ( int i = 0; i < MailCommon::StatusValueCountWithoutHidden; ++i ) {
    if ( MailCommon::StatusValues[ i ].icon != 0 ) {
      statusCombo->addItem(
        SmallIcon( QLatin1String(MailCommon::StatusValues[ i ].icon) ),
        i18nc( "message status", MailCommon::StatusValues[ i ].text ) );
    } else {
      statusCombo->addItem(
        i18nc( "message status", MailCommon::StatusValues[ i ].text ) );
    }
  }
  statusCombo->adjustSize();
  QObject::connect( statusCombo, SIGNAL(activated(int)),
                    receiver, SLOT(slotValueChanged()) );
  return statusCombo;
}

//---------------------------------------------------------------------------

SearchRule::Function StatusRuleWidgetHandler::currentFunction(
  const QStackedWidget *functionStack ) const
{
  const PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleFuncCombo") );

  if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
    return StatusFunctions[funcCombo->currentIndex()].id;
  }

  return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function StatusRuleWidgetHandler::function( const QByteArray &field,
                                                        const QStackedWidget *functionStack ) const
{
  if ( !handlesField( field ) ) {
    return SearchRule::FuncNone;
  }

  return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

int StatusRuleWidgetHandler::currentStatusValue( const QStackedWidget *valueStack ) const
{
  const PimCommon::MinimumComboBox *statusCombo =
    valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleValueCombo") );

  if ( statusCombo ) {
    return statusCombo->currentIndex();
  }

  return -1;
}

//---------------------------------------------------------------------------

QString StatusRuleWidgetHandler::value( const QByteArray &field,
                                        const QStackedWidget *,
                                        const QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return QString();
  }

  const int status = currentStatusValue( valueStack );
  if ( status != -1 ) {
    return QString::fromLatin1( MailCommon::StatusValues[ status ].text );
  } else {
    return QString();
  }
}

//---------------------------------------------------------------------------

QString StatusRuleWidgetHandler::prettyValue( const QByteArray &field,
                                              const QStackedWidget *,
                                              const QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return QString();
  }

  const int status = currentStatusValue( valueStack );
  if ( status != -1 ) {
    return i18nc( "message status", MailCommon::StatusValues[ status ].text );
  } else {
    return QString();
  }
}

//---------------------------------------------------------------------------

bool StatusRuleWidgetHandler::handlesField( const QByteArray &field ) const
{
  return ( field == "<status>" );
}

//---------------------------------------------------------------------------

void StatusRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                     QStackedWidget *valueStack ) const
{
  // reset the function combo box
  PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleFuncCombo") );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the status value combo box
  PimCommon::MinimumComboBox *statusCombo =
    valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleValueCombo") );

  if ( statusCombo ) {
    statusCombo->blockSignals( true );
    statusCombo->setCurrentIndex( 0 );
    statusCombo->blockSignals( false );
  }
}

//---------------------------------------------------------------------------

bool StatusRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                       QStackedWidget *valueStack,
                                       const SearchRule::Ptr rule ) const
{
  if ( !rule || !handlesField( rule->field() ) ) {
    reset( functionStack, valueStack );
    return false;
  }

  // set the function
  const SearchRule::Function func = rule->function();
  int funcIndex = 0;
  for ( ; funcIndex < StatusFunctionCount; ++funcIndex ) {
    if ( func == StatusFunctions[funcIndex].id ) {
      break;
    }
  }

  PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleFuncCombo") );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    if ( funcIndex < StatusFunctionCount ) {
      funcCombo->setCurrentIndex( funcIndex );
    } else {
      funcCombo->setCurrentIndex( 0 );
    }
    funcCombo->blockSignals( false );
    functionStack->setCurrentWidget( funcCombo );
  }

  // set the value
  const QString value = rule->contents();
  int valueIndex = 0;
  for ( ; valueIndex <MailCommon::StatusValueCountWithoutHidden; ++valueIndex ) {
    if ( value == QString::fromLatin1( MailCommon::StatusValues[ valueIndex ].text ) ) {
      break;
    }
  }

  PimCommon::MinimumComboBox *statusCombo =
    valueStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("statusRuleValueCombo") );

  if ( statusCombo ) {
    statusCombo->blockSignals( true );
    if ( valueIndex <MailCommon::StatusValueCountWithoutHidden ) {
      statusCombo->setCurrentIndex( valueIndex );
    } else {
      statusCombo->setCurrentIndex( 0 );
    }
    statusCombo->blockSignals( false );
    valueStack->setCurrentWidget( statusCombo );
  }
  return true;
}

//---------------------------------------------------------------------------

bool StatusRuleWidgetHandler::update( const QByteArray &field,
                                      QStackedWidget *functionStack,
                                      QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return false;
  }

  // raise the correct function widget
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>(QLatin1String( "statusRuleFuncCombo") ) );

  // raise the correct value widget
  valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( QLatin1String("statusRuleValueCombo") ) );

  return true;
}

} // anonymous namespace for StatusRuleWidgetHandler


//=============================================================================
//
// class TagRuleWidgetHandler
//
//=============================================================================

namespace {

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
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
  funcCombo->setObjectName( QLatin1String("tagRuleFuncCombo") );
  for ( int i = 0; i < TagFunctionCount; ++i ) {
    funcCombo->addItem( i18n( TagFunctions[i].displayName ) );
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
                                    const SearchRule::Ptr rule ) const
{
  if ( !rule || !handlesField( rule->field() ) ) {
    reset( functionStack, valueStack );
    return false;
  }

  // set the function
  const SearchRule::Function func = rule->function();
  int funcIndex = 0;
  for ( ; funcIndex < StatusFunctionCount; ++funcIndex ) {
    if ( func == StatusFunctions[funcIndex].id ) {
      break;
    }
  }

  PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("tagRuleFuncCombo") );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    if ( funcIndex < StatusFunctionCount ) {
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

} // anonymous namespace for TagRuleWidgetHandler


//=============================================================================
//
// class NumericRuleWidgetHandler
//
//=============================================================================

namespace {

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

//---------------------------------------------------------------------------

QWidget *NumericRuleWidgetHandler::createFunctionWidget(
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  PimCommon::MinimumComboBox *funcCombo = new PimCommon::MinimumComboBox( functionStack );
  funcCombo->setObjectName( QLatin1String("numericRuleFuncCombo") );
  for ( int i = 0; i < NumericFunctionCount; ++i ) {
    funcCombo->addItem( i18n( NumericFunctions[i].displayName ) );
  }
  funcCombo->adjustSize();
  QObject::connect( funcCombo, SIGNAL(activated(int)),
                    receiver, SLOT(slotFunctionChanged()) );
  return funcCombo;
}

//---------------------------------------------------------------------------

QWidget *NumericRuleWidgetHandler::createValueWidget( int number,
                                                      QStackedWidget *valueStack,
                                                      const QObject *receiver ) const
{
  if ( number != 0 ) {
    return 0;
  }

  KIntNumInput *numInput = new KIntNumInput( valueStack );
  numInput->setSliderEnabled( false );
  numInput->setObjectName( QLatin1String("KIntNumInput") );
  QObject::connect( numInput, SIGNAL(valueChanged(int)),
                    receiver, SLOT(slotValueChanged()) );
  return numInput;
}

//---------------------------------------------------------------------------

SearchRule::Function NumericRuleWidgetHandler::currentFunction(
  const QStackedWidget *functionStack ) const
{
  const PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericRuleFuncCombo") );

  if ( funcCombo && funcCombo->currentIndex() >= 0 ) {
    return NumericFunctions[funcCombo->currentIndex()].id;
  }

  return SearchRule::FuncNone;
}

//---------------------------------------------------------------------------

SearchRule::Function NumericRuleWidgetHandler::function( const QByteArray &field,
                                                         const QStackedWidget *functionStack ) const
{
  if ( !handlesField( field ) ) {
    return SearchRule::FuncNone;
  }

  return currentFunction( functionStack );
}

//---------------------------------------------------------------------------

QString NumericRuleWidgetHandler::currentValue( const QStackedWidget *valueStack ) const
{
  const KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( QLatin1String("KIntNumInput") );

  if ( numInput ) {
    return QString::number( numInput->value() );
  }

  return QString();
}

//---------------------------------------------------------------------------

QString NumericRuleWidgetHandler::value( const QByteArray &field,
                                         const QStackedWidget *,
                                         const QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return QString();
  }

  return currentValue( valueStack );
}

//---------------------------------------------------------------------------

QString NumericRuleWidgetHandler::prettyValue( const QByteArray &field,
                                               const QStackedWidget *,
                                               const QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return QString();
  }

  return currentValue( valueStack );
}

//---------------------------------------------------------------------------

bool NumericRuleWidgetHandler::handlesField( const QByteArray &field ) const
{
  return field == "<age in days>";
}

//---------------------------------------------------------------------------

void NumericRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                      QStackedWidget *valueStack ) const
{
  // reset the function combo box
  PimCommon::MinimumComboBox *funcCombo =
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericRuleFuncCombo") );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the value widget
  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( QLatin1String("KIntNumInput") );

  if ( numInput ) {
    numInput->blockSignals( true );
    numInput->setValue( 0 );
    numInput->blockSignals( false );
  }
}

//---------------------------------------------------------------------------

void initNumInput( KIntNumInput *numInput, const QByteArray &field )
{
  if ( field == "<age in days>" ) {
    numInput->setMinimum( -10000 );
    numInput->setSuffix( i18nc( "Unit suffix where units are days.", " days" ) );
    numInput->setSliderEnabled( false );
  }
}

//---------------------------------------------------------------------------

bool NumericRuleWidgetHandler::setRule( QStackedWidget *functionStack,
                                        QStackedWidget *valueStack,
                                        const SearchRule::Ptr rule ) const
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
    functionStack->findChild<PimCommon::MinimumComboBox*>( QLatin1String("numericRuleFuncCombo") );

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

  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( QLatin1String("KIntNumInput") );

  if ( numInput ) {
    initNumInput( numInput, rule->field() );
    numInput->blockSignals( true );
    numInput->setValue( value );
    numInput->blockSignals( false );
    valueStack->setCurrentWidget( numInput );
  }
  return true;
}

//---------------------------------------------------------------------------

bool NumericRuleWidgetHandler::update( const QByteArray &field,
                                       QStackedWidget *functionStack,
                                       QStackedWidget *valueStack ) const
{
  if ( !handlesField( field ) ) {
    return false;
  }

  // raise the correct function widget
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( QLatin1String("numericRuleFuncCombo") ) );

  // raise the correct value widget
  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( QLatin1String("KIntNumInput") );

  if ( numInput ) {
    initNumInput( numInput, field );
    valueStack->setCurrentWidget( numInput );
  }
  return true;
}
}

//***//
//=============================================================================
//
// class DateRuleWidgetHandler
//
//=============================================================================

namespace {

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
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
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
                                        const SearchRule::Ptr rule ) const
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
} // anonymous namespace for DateRuleWidgetHandler




//=============================================================================
//
// class NumericDoubleRuleWidgetHandler
//
//=============================================================================

namespace {

//---------------------------------------------------------------------------

QWidget *NumericDoubleRuleWidgetHandler::createFunctionWidget(
  int number, QStackedWidget *functionStack, const QObject *receiver ) const
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
                                        const SearchRule::Ptr rule ) const
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
}

