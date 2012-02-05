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
#include "minimumcombobox.h"
#include "interfaces/rulewidgethandler.h"

#include <messageviewer/stl_util.h>

#ifndef KDEPIM_NO_NEPOMUK
#include <Nepomuk/Tag>
#endif

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

#ifndef KDEPIM_NO_NEPOMUK

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

#endif

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

}

MailCommon::RuleWidgetHandlerManager::RuleWidgetHandlerManager()
{
#ifndef KDEPIM_NO_NEPOMUK
  registerHandler( new TagRuleWidgetHandler() );
#endif

  registerHandler( new NumericRuleWidgetHandler() );
  registerHandler( new StatusRuleWidgetHandler() );
  registerHandler( new MessageRuleWidgetHandler() );
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
  assert( rule );
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
#include "searchpattern.h"
#include "regexplineedit.h"
using MailCommon::RegExpLineEdit;

#include <KLocale>
#include <KNumInput>

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
#ifndef KDEPIM_NO_NEPOMUK
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

  MinimumComboBox *funcCombo = new MinimumComboBox( functionStack );
  funcCombo->setObjectName( "textRuleFuncCombo" );
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
    lineEdit->setObjectName( "regExpLineEdit" );
    QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                      receiver, SLOT(slotValueChanged()) );
    return lineEdit;
  }

  // blank QLabel to hide value widget for in-address-book rule
  if ( number == 1 ) {
    QLabel *label = new QLabel( valueStack );
    label->setObjectName( "textRuleValueHider" );
    label->setBuddy( valueStack );
    return label;
  }

//FIXME: review what is this about, why is nepomuk used
#ifndef KDEPIM_NO_NEPOMUK

  if ( number == 2 ) {
    MinimumComboBox *combo =  new MinimumComboBox( valueStack );
    combo->setObjectName( "categoryCombo" );
    foreach ( const Nepomuk::Tag &tag, Nepomuk::Tag::allTags() ) {
      if ( tag.genericIcon().isEmpty() ) {
        combo->addItem( tag.label(), tag.resourceUri() );
      } else {
        combo->addItem( KIcon( tag.genericIcon() ), tag.label(), tag.resourceUri() );
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
  const MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( QString( "textRuleFuncCombo" ) );

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
#ifndef KDEPIM_NO_NEPOMUK
  // here we gotta check the combobox which contains the categories
  if ( func  == SearchRule::FuncIsInCategory ||
       func  == SearchRule::FuncIsNotInCategory ) {
    const MinimumComboBox *combo = valueStack->findChild<MinimumComboBox*>( "categoryCombo" );

    if ( combo ) {
      return combo->currentText();
    } else {
      return QString();
    }
  }
#endif

  //in other cases of func it is a lineedit
  const RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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
  MinimumComboBox *funcCombo = functionStack->findChild<MinimumComboBox*>( "textRuleFuncCombo" );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the value widget
  RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );
  if ( lineEdit ) {
    lineEdit->blockSignals( true );
    lineEdit->clear();
    lineEdit->blockSignals( false );
    lineEdit->showEditButton( false );
    valueStack->setCurrentWidget( lineEdit );
  }

  MinimumComboBox *combo = valueStack->findChild<MinimumComboBox*>( "categoryCombo" );

  if ( combo ) {
    combo->blockSignals( true );
    combo->setCurrentIndex( 0 );
    combo->blockSignals( false );
  }
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

  MinimumComboBox *funcCombo = functionStack->findChild<MinimumComboBox*>( "textRuleFuncCombo" );

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
    QWidget *w = valueStack->findChild<QWidget*>( "textRuleValueHider" );
    valueStack->setCurrentWidget( w );
  }
#ifndef KDEPIM_NO_NEPOMUK
  else if ( func == SearchRule::FuncIsInCategory ||
            func == SearchRule::FuncIsNotInCategory ) {
    MinimumComboBox *combo = valueStack->findChild<MinimumComboBox*>( "categoryCombo" );

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
      valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( "textRuleFuncCombo" ) );

  // raise the correct value widget
  SearchRule::Function func = currentFunction( functionStack );
  if ( func == SearchRule::FuncIsInAddressbook ||
       func == SearchRule::FuncIsNotInAddressbook ) {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( "textRuleValueHider" ) );
  }
#ifndef KDEPIM_NO_NEPOMUK
  else if ( func == SearchRule::FuncIsInCategory ||
            func == SearchRule::FuncIsNotInCategory ) {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( "categoryCombo" ) );
  }
#endif
  else {
    RegExpLineEdit *lineEdit =
      valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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

  MinimumComboBox *funcCombo = new MinimumComboBox( functionStack );
  funcCombo->setObjectName( "messageRuleFuncCombo" );
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
    lineEdit->setObjectName( "regExpLineEdit" );
    QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                      receiver, SLOT(slotValueChanged()) );
    return lineEdit;
  }

  // blank QLabel to hide value widget for has-attachment rule
  if ( number == 1 ) {
    QLabel *label = new QLabel( valueStack );
    label->setObjectName( "textRuleValueHider" );
    label->setBuddy( valueStack );
    return label;
  }

  return 0;
}

//---------------------------------------------------------------------------

SearchRule::Function MessageRuleWidgetHandler::currentFunction(
  const QStackedWidget *functionStack ) const
{
  const MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "messageRuleFuncCombo" );

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
  const RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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
    return "has an attachment"; // just a non-empty dummy value
  } else if ( func == SearchRule::FuncHasNoAttachment ) {
    return "has no attachment"; // just a non-empty dummy value
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
  MinimumComboBox *funcCombo = functionStack->findChild<MinimumComboBox*>( "messageRuleFuncCombo" );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the value widget
  RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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

  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "messageRuleFuncCombo" );

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
    QWidget *w = valueStack->findChild<QWidget*>( "textRuleValueHider" );
    valueStack->setCurrentWidget( w );
  } else {
    RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( "messageRuleFuncCombo" ) );

  // raise the correct value widget
  SearchRule::Function func = currentFunction( functionStack );
  if ( func == SearchRule::FuncHasAttachment  ||
       func == SearchRule::FuncHasNoAttachment ) {
    QWidget *w = valueStack->findChild<QWidget*>( "textRuleValueHider" );
    valueStack->setCurrentWidget( w );
  } else {
    RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "regExpLineEdit" );

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

  MinimumComboBox *funcCombo = new MinimumComboBox( functionStack );
  funcCombo->setObjectName( "statusRuleFuncCombo" );
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

  MinimumComboBox *statusCombo = new MinimumComboBox( valueStack );
  statusCombo->setObjectName( "statusRuleValueCombo" );
  for ( int i = 0; i < MailCommon::StatusValueCountWithoutHidden; ++i ) {
    if ( MailCommon::StatusValues[ i ].icon != 0 ) {
      statusCombo->addItem(
        SmallIcon( MailCommon::StatusValues[ i ].icon ),
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
  const MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "statusRuleFuncCombo" );

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
  const MinimumComboBox *statusCombo =
    valueStack->findChild<MinimumComboBox*>( "statusRuleValueCombo" );

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
  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "statusRuleFuncCombo" );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the status value combo box
  MinimumComboBox *statusCombo =
    valueStack->findChild<MinimumComboBox*>( "statusRuleValueCombo" );

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

  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "statusRuleFuncCombo" );

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

  MinimumComboBox *statusCombo =
    valueStack->findChild<MinimumComboBox*>( "statusRuleValueCombo" );

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
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( "statusRuleFuncCombo" ) );

  // raise the correct value widget
  valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( "statusRuleValueCombo" ) );

  return true;
}

} // anonymous namespace for StatusRuleWidgetHandler

#ifndef KDEPIM_NO_NEPOMUK

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

  MinimumComboBox *funcCombo = new MinimumComboBox( functionStack );
  funcCombo->setObjectName( "tagRuleFuncCombo" );
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
    lineEdit->setObjectName( "tagRuleRegExpLineEdit" );
    QObject::connect( lineEdit, SIGNAL(textChanged(QString)),
                      receiver, SLOT(slotValueChanged()) );
    return lineEdit;
  }

  if ( number == 1 ) {
    MinimumComboBox *valueCombo = new MinimumComboBox( valueStack );
    valueCombo->setObjectName( "tagRuleValueCombo" );
    valueCombo->setEditable( true );
    valueCombo->addItem( QString() ); // empty entry for user input
    foreach ( const Nepomuk::Tag &tag, Nepomuk::Tag::allTags() ) {
      QString iconName( "mail-tagged" );
      if ( !tag.symbols().isEmpty() ) {
        iconName = tag.symbols().first();
      }
      valueCombo->addItem( KIcon( iconName ), tag.label(), tag.resourceUri() );
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

  const MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "tagRuleFuncCombo" );

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
      valueStack->findChild<RegExpLineEdit*>( "tagRuleRegExpLineEdit " );

    if ( lineEdit ) {
      return lineEdit->text();
    } else {
      return QString();
    }
  }

  // Use combo box
  const MinimumComboBox *tagCombo =
    valueStack->findChild<MinimumComboBox*>( "tagRuleValueCombo" );

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
  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "tagRuleFuncCombo" );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the status value combo box and reg exp line edit
  RegExpLineEdit *lineEdit =
    valueStack->findChild<RegExpLineEdit*>( "tagRuleRegExpLineEdit" );

  if ( lineEdit ) {
    lineEdit->blockSignals( true );
    lineEdit->clear();
    lineEdit->blockSignals( false );
    lineEdit->showEditButton( false );
    valueStack->setCurrentWidget( lineEdit );
  }

  MinimumComboBox *tagCombo = valueStack->findChild<MinimumComboBox*>( "tagRuleValueCombo" );
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

  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "tagRuleFuncCombo" );

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
    RegExpLineEdit *lineEdit = valueStack->findChild<RegExpLineEdit*>( "tagRuleRegExpLineEdit" );

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
    foreach ( const Nepomuk::Tag &tag, Nepomuk::Tag::allTags() ) {
      if ( tag.label() == rule->contents() ) {
        valueIndex = tagIndex;
        break;
      }
      tagIndex++;
    }

    MinimumComboBox *tagCombo =
      valueStack->findChild<MinimumComboBox*>( "tagRuleValueCombo" );

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
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( "tagRuleFuncCombo" ) );

  // raise the correct value widget
  SearchRule::Function func = function( field, functionStack );
  if ( func == SearchRule::FuncRegExp || func == SearchRule::FuncNotRegExp ) {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( "tagRuleRegExpLineEdit" ) );
  } else {
    valueStack->setCurrentWidget( valueStack->findChild<QWidget*>( "tagRuleValueCombo" ) );
  }

  return true;
}

} // anonymous namespace for TagRuleWidgetHandler

#endif

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

  MinimumComboBox *funcCombo = new MinimumComboBox( functionStack );
  funcCombo->setObjectName( "numericRuleFuncCombo" );
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
  numInput->setObjectName( "KIntNumInput" );
  QObject::connect( numInput, SIGNAL(valueChanged(int)),
                    receiver, SLOT(slotValueChanged()) );
  return numInput;
}

//---------------------------------------------------------------------------

SearchRule::Function NumericRuleWidgetHandler::currentFunction(
  const QStackedWidget *functionStack ) const
{
  const MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "numericRuleFuncCombo" );

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
  const KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( "KIntNumInput" );

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
  return ( field == "<size>" || field == "<age in days>" );
}

//---------------------------------------------------------------------------

void NumericRuleWidgetHandler::reset( QStackedWidget *functionStack,
                                      QStackedWidget *valueStack ) const
{
  // reset the function combo box
  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "numericRuleFuncCombo" );

  if ( funcCombo ) {
    funcCombo->blockSignals( true );
    funcCombo->setCurrentIndex( 0 );
    funcCombo->blockSignals( false );
  }

  // reset the value widget
  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( "KIntNumInput" );

  if ( numInput ) {
    numInput->blockSignals( true );
    numInput->setValue( 0 );
    numInput->blockSignals( false );
  }
}

//---------------------------------------------------------------------------

void initNumInput( KIntNumInput *numInput, const QByteArray &field )
{
  if ( field == "<size>" ) {
    numInput->setMinimum( 0 );
    numInput->setSuffix( i18n( " bytes" ) );
    numInput->setSliderEnabled( false );
  } else {
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

  MinimumComboBox *funcCombo =
    functionStack->findChild<MinimumComboBox*>( "numericRuleFuncCombo" );

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

  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( "KIntNumInput" );

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
  functionStack->setCurrentWidget( functionStack->findChild<QWidget*>( "numericRuleFuncCombo" ) );

  // raise the correct value widget
  KIntNumInput *numInput = valueStack->findChild<KIntNumInput*>( "KIntNumInput" );

  if ( numInput ) {
    initNumInput( numInput, field );
    valueStack->setCurrentWidget( numInput );
  }
  return true;
}

} // anonymous namespace for NumericRuleWidgetHandler

