/*  -*- mode: C++; c-file-style: "gnu" -*-
    rulewidgethandlermanager.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Ingo Kloecker <kloecker@kde.org>

    KMail is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rulewidgethandlermanager.h"

#include "interfaces/rulewidgethandler.h"
#include "stl_util.h"

#include <kdebug.h>
#include <kiconloader.h>

#include <tqwidgetstack.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqobject.h>
#include <tqobjectlist.h>

#include <assert.h>

#include <algorithm>
using std::for_each;
using std::remove;

KMail::RuleWidgetHandlerManager * KMail::RuleWidgetHandlerManager::self = 0;

namespace {
  class TextRuleWidgetHandler : public KMail::RuleWidgetHandler {
  public:
    TextRuleWidgetHandler() : KMail::RuleWidgetHandler() {}
    ~TextRuleWidgetHandler() {}

    TQWidget * createFunctionWidget( int number,
                                    TQWidgetStack *functionStack,
                                    const TQObject *receiver ) const;
    TQWidget * createValueWidget( int number,
                                 TQWidgetStack *valueStack,
                                 const TQObject *receiver ) const;
    KMSearchRule::Function function( const TQCString & field,
                                     const TQWidgetStack *functionStack ) const;
    TQString value( const TQCString & field,
                   const TQWidgetStack *functionStack,
                   const TQWidgetStack *valueStack ) const;
    TQString prettyValue( const TQCString & field,
                         const TQWidgetStack *functionStack,
                         const TQWidgetStack *valueStack ) const;
    bool handlesField( const TQCString & field ) const;
    void reset( TQWidgetStack *functionStack,
                TQWidgetStack *valueStack ) const;
    bool setRule( TQWidgetStack *functionStack,
                  TQWidgetStack *valueStack,
                  const KMSearchRule *rule ) const;
    bool update( const TQCString & field,
                 TQWidgetStack *functionStack,
                 TQWidgetStack *valueStack ) const;

 private:
    KMSearchRule::Function currentFunction( const TQWidgetStack *functionStack ) const;
    TQString currentValue( const TQWidgetStack *valueStack,
                          KMSearchRule::Function func ) const;
  };

  class MessageRuleWidgetHandler : public KMail::RuleWidgetHandler {
  public:
    MessageRuleWidgetHandler() : KMail::RuleWidgetHandler() {}
    ~MessageRuleWidgetHandler() {}

    TQWidget * createFunctionWidget( int number,
                                    TQWidgetStack *functionStack,
                                    const TQObject *receiver ) const;
    TQWidget * createValueWidget( int number,
                                 TQWidgetStack *valueStack,
                                 const TQObject *receiver ) const;
    KMSearchRule::Function function( const TQCString & field,
                                     const TQWidgetStack *functionStack ) const;
    TQString value( const TQCString & field,
                   const TQWidgetStack *functionStack,
                   const TQWidgetStack *valueStack ) const;
    TQString prettyValue( const TQCString & field,
                         const TQWidgetStack *functionStack,
                         const TQWidgetStack *valueStack ) const;
    bool handlesField( const TQCString & field ) const;
    void reset( TQWidgetStack *functionStack,
                TQWidgetStack *valueStack ) const;
    bool setRule( TQWidgetStack *functionStack,
                  TQWidgetStack *valueStack,
                  const KMSearchRule *rule ) const;
    bool update( const TQCString & field,
                 TQWidgetStack *functionStack,
                 TQWidgetStack *valueStack ) const;

 private:
    KMSearchRule::Function currentFunction( const TQWidgetStack *functionStack ) const;
    TQString currentValue( const TQWidgetStack *valueStack,
                          KMSearchRule::Function func ) const;
  };


  class StatusRuleWidgetHandler : public KMail::RuleWidgetHandler {
  public:
    StatusRuleWidgetHandler() : KMail::RuleWidgetHandler() {}
    ~StatusRuleWidgetHandler() {}

    TQWidget * createFunctionWidget( int number,
                                    TQWidgetStack *functionStack,
                                    const TQObject *receiver ) const;
    TQWidget * createValueWidget( int number,
                                 TQWidgetStack *valueStack,
                                 const TQObject *receiver ) const;
    KMSearchRule::Function function( const TQCString & field,
                                     const TQWidgetStack *functionStack ) const;
    TQString value( const TQCString & field,
                   const TQWidgetStack *functionStack,
                   const TQWidgetStack *valueStack ) const;
    TQString prettyValue( const TQCString & field,
                         const TQWidgetStack *functionStack,
                         const TQWidgetStack *valueStack ) const;
    bool handlesField( const TQCString & field ) const;
    void reset( TQWidgetStack *functionStack,
                TQWidgetStack *valueStack ) const;
    bool setRule( TQWidgetStack *functionStack,
                  TQWidgetStack *valueStack,
                  const KMSearchRule *rule ) const;
    bool update( const TQCString & field,
                 TQWidgetStack *functionStack,
                 TQWidgetStack *valueStack ) const;

  private:
    KMSearchRule::Function currentFunction( const TQWidgetStack *functionStack ) const;
    int currentStatusValue( const TQWidgetStack *valueStack ) const;
  };

  class NumericRuleWidgetHandler : public KMail::RuleWidgetHandler {
  public:
    NumericRuleWidgetHandler() : KMail::RuleWidgetHandler() {}
    ~NumericRuleWidgetHandler() {}

    TQWidget * createFunctionWidget( int number,
                                    TQWidgetStack *functionStack,
                                    const TQObject *receiver ) const;
    TQWidget * createValueWidget( int number,
                                 TQWidgetStack *valueStack,
                                 const TQObject *receiver ) const;
    KMSearchRule::Function function( const TQCString & field,
                                     const TQWidgetStack *functionStack ) const;
    TQString value( const TQCString & field,
                   const TQWidgetStack *functionStack,
                   const TQWidgetStack *valueStack ) const;
    TQString prettyValue( const TQCString & field,
                         const TQWidgetStack *functionStack,
                         const TQWidgetStack *valueStack ) const;
    bool handlesField( const TQCString & field ) const;
    void reset( TQWidgetStack *functionStack,
                TQWidgetStack *valueStack ) const;
    bool setRule( TQWidgetStack *functionStack,
                  TQWidgetStack *valueStack,
                  const KMSearchRule *rule ) const;
    bool update( const TQCString & field,
                 TQWidgetStack *functionStack,
                 TQWidgetStack *valueStack ) const;

  private:
    KMSearchRule::Function currentFunction( const TQWidgetStack *functionStack ) const;
    TQString currentValue( const TQWidgetStack *valueStack ) const;
  };
}

KMail::RuleWidgetHandlerManager::RuleWidgetHandlerManager()
{
  registerHandler( new NumericRuleWidgetHandler() );
  registerHandler( new StatusRuleWidgetHandler() );
  registerHandler( new MessageRuleWidgetHandler() );
   // the TextRuleWidgetHandler is the fallback handler, so it has to be added
  // as last handler
  registerHandler( new TextRuleWidgetHandler() );
}

KMail::RuleWidgetHandlerManager::~RuleWidgetHandlerManager()
{
  for_each( mHandlers.begin(), mHandlers.end(),
	    DeleteAndSetToZero<RuleWidgetHandler>() );
}

void KMail::RuleWidgetHandlerManager::registerHandler( const RuleWidgetHandler * handler )
{
  if ( !handler )
    return;
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void KMail::RuleWidgetHandlerManager::unregisterHandler( const RuleWidgetHandler * handler )
{
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

namespace {
  /** Returns the number of immediate children of parent with the given object
      name. Used by RuleWidgetHandlerManager::createWidgets().
  */
  int childCount( const TQObject *parent, const char *objName )
  {
    TQObjectList *list = parent->queryList( 0, objName, false, false );
    if ( !list )
      return 0;
    const int count = list->count();
    delete list; list = 0;
    return count;
  }
}

void KMail::RuleWidgetHandlerManager::createWidgets( TQWidgetStack *functionStack,
                                                     TQWidgetStack *valueStack,
                                                     const TQObject *receiver ) const
{
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    TQWidget *w = 0;
    for ( int i = 0;
          ( w = (*it)->createFunctionWidget( i, functionStack, receiver ) );
          ++i ) {
      if ( childCount( functionStack, w->name() ) < 2 ) {
        // there wasn't already a widget with this name, so add this widget
        functionStack->addWidget( w );
      }
      else {
        // there was already a widget with this name, so discard this widget
        kdDebug(5006) << "RuleWidgetHandlerManager::createWidgets: "
                      << w->name() << " already exists in functionStack"
                      << endl;
        delete w; w = 0;
      }
    }
    for ( int i = 0;
          ( w = (*it)->createValueWidget( i, valueStack, receiver ) );
          ++i ) {
      if ( childCount( valueStack, w->name() ) < 2 ) {
        // there wasn't already a widget with this name, so add this widget
        valueStack->addWidget( w );
      }
      else {
        // there was already a widget with this name, so discard this widget
        kdDebug(5006) << "RuleWidgetHandlerManager::createWidgets: "
                      << w->name() << " already exists in valueStack"
                      << endl;
        delete w; w = 0;
      }
    }
  }
}

KMSearchRule::Function KMail::RuleWidgetHandlerManager::function( const TQCString& field,
                                                                  const TQWidgetStack *functionStack ) const
{
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    const KMSearchRule::Function func = (*it)->function( field,
                                                         functionStack );
    if ( func != KMSearchRule::FuncNone )
      return func;
  }
  return KMSearchRule::FuncNone;
}

TQString KMail::RuleWidgetHandlerManager::value( const TQCString& field,
                                                const TQWidgetStack *functionStack,
                                                const TQWidgetStack *valueStack ) const
{
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    const TQString val = (*it)->value( field, functionStack, valueStack );
    if ( !val.isEmpty() )
      return val;
  }
  return TQString::null;
}

TQString KMail::RuleWidgetHandlerManager::prettyValue( const TQCString& field,
                                                      const TQWidgetStack *functionStack,
                                                      const TQWidgetStack *valueStack ) const
{
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    const TQString val = (*it)->prettyValue( field, functionStack, valueStack );
    if ( !val.isEmpty() )
      return val;
  }
  return TQString::null;
}

void KMail::RuleWidgetHandlerManager::reset( TQWidgetStack *functionStack,
                                             TQWidgetStack *valueStack ) const
{
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    (*it)->reset( functionStack, valueStack );
  }
  update( "", functionStack, valueStack );
}

void KMail::RuleWidgetHandlerManager::setRule( TQWidgetStack *functionStack,
                                               TQWidgetStack *valueStack,
                                               const KMSearchRule *rule ) const
{
  assert( rule );
  reset( functionStack, valueStack );
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    if ( (*it)->setRule( functionStack, valueStack, rule ) )
      return;
  }
}

void KMail::RuleWidgetHandlerManager::update( const TQCString &field,
                                              TQWidgetStack *functionStack,
                                              TQWidgetStack *valueStack ) const
{
  //kdDebug(5006) << "RuleWidgetHandlerManager::update( \"" << field
  //              << "\", ... )" << endl;
  for ( const_iterator it = mHandlers.begin(); it != mHandlers.end(); ++it ) {
    if ( (*it)->update( field, functionStack, valueStack ) )
      return;
  }
}

//-----------------------------------------------------------------------------

namespace {
  // FIXME (Qt >= 4.0):
  // This is a simplified and constified copy of TQObject::child(). According
  // to a comment in tqobject.h TQObject::child() will be made const in Qt 4.0.
  // So once we require Qt 4.0 this can be removed.
  TQObject* QObject_child_const( const TQObject *parent,
                                const char *objName )
  {
    const TQObjectList *list = parent->children();
    if ( !list )
      return 0;

    TQObjectListIterator it( *list );
    TQObject *obj;
    while ( ( obj = it.current() ) ) {
      ++it;
      if ( !objName || qstrcmp( objName, obj->name() ) == 0 )
        break;
    }
    return obj;
  }
}

//-----------------------------------------------------------------------------

// these includes are temporary and should not be needed for the code
// above this line, so they appear only here:
#include "kmaddrbook.h"
#include "kmsearchpattern.h"
#include "regexplineedit.h"
using KMail::RegExpLineEdit;

#include <klocale.h>
#include <knuminput.h>

#include <tqcombobox.h>
#include <tqlabel.h>

//=============================================================================
//
// class TextRuleWidgetHandler
//
//=============================================================================

namespace {
  // also see KMSearchRule::matches() and KMSearchRule::Function
  // if you change the following strings!
  static const struct {
    const KMSearchRule::Function id;
    const char *displayName;
  } TextFunctions[] = {
    { KMSearchRule::FuncContains,           I18N_NOOP( "contains" )          },
    { KMSearchRule::FuncContainsNot,        I18N_NOOP( "does not contain" )   },
    { KMSearchRule::FuncEquals,             I18N_NOOP( "equals" )            },
    { KMSearchRule::FuncNotEqual,           I18N_NOOP( "does not equal" )     },
    { KMSearchRule::FuncRegExp,             I18N_NOOP( "matches regular expr." ) },
    { KMSearchRule::FuncNotRegExp,          I18N_NOOP( "does not match reg. expr." ) },
    { KMSearchRule::FuncIsInAddressbook,    I18N_NOOP( "is in address book" ) },
    { KMSearchRule::FuncIsNotInAddressbook, I18N_NOOP( "is not in address book" ) },
    { KMSearchRule::FuncIsInCategory,       I18N_NOOP( "is in category" ) },
    { KMSearchRule::FuncIsNotInCategory,    I18N_NOOP( "is not in category" ) }
  };
  static const int TextFunctionCount =
    sizeof( TextFunctions ) / sizeof( *TextFunctions );

  //---------------------------------------------------------------------------

  TQWidget * TextRuleWidgetHandler::createFunctionWidget( int number,
                                                         TQWidgetStack *functionStack,
                                                         const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    TQComboBox *funcCombo = new TQComboBox( functionStack, "textRuleFuncCombo" );
    for ( int i = 0; i < TextFunctionCount; ++i ) {
      funcCombo->insertItem( i18n( TextFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    TQObject::connect( funcCombo, TQT_SIGNAL( activated( int ) ),
                      receiver, TQT_SLOT( slotFunctionChanged() ) );
    return funcCombo;
  }

  //---------------------------------------------------------------------------

  TQWidget * TextRuleWidgetHandler::createValueWidget( int number,
                                                      TQWidgetStack *valueStack,
                                                      const TQObject *receiver ) const
  {
    if ( number == 0 ) {
      RegExpLineEdit *lineEdit =
        new RegExpLineEdit( valueStack, "regExpLineEdit" );
      TQObject::connect( lineEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
                        receiver, TQT_SLOT( slotValueChanged() ) );
      return lineEdit;
    }

    // blank TQLabel to hide value widget for in-address-book rule
    if ( number == 1 ) {
      return new TQLabel( valueStack, "textRuleValueHider" );
    }

    if ( number == 2 ) {
      TQComboBox *combo =  new TQComboBox( valueStack, "categoryCombo" );
      TQStringList categories = KabcBridge::categories();
      combo->insertStringList( categories );
      TQObject::connect( combo, TQT_SIGNAL( activated( int ) ),
                        receiver, TQT_SLOT( slotValueChanged() ) );
      return combo;
    }

    return 0;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function TextRuleWidgetHandler::currentFunction( const TQWidgetStack *functionStack ) const
  {
    const TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( QObject_child_const( functionStack,
                                                     "textRuleFuncCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( functionStack->child( "textRuleFuncCombo",
    //                                                  0, false ) );
    if ( funcCombo ) {
      return TextFunctions[funcCombo->currentItem()].id;
    }
    else
      kdDebug(5006) << "TextRuleWidgetHandler::currentFunction: "
                       "textRuleFuncCombo not found." << endl;
    return KMSearchRule::FuncNone;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function TextRuleWidgetHandler::function( const TQCString &,
                                                          const TQWidgetStack *functionStack ) const
  {
    return currentFunction( functionStack );
  }

  //---------------------------------------------------------------------------

  TQString TextRuleWidgetHandler::currentValue( const TQWidgetStack *valueStack,
                                               KMSearchRule::Function func ) const
  {
    // here we gotta check the combobox which contains the categories
    if ( func  == KMSearchRule::FuncIsInCategory ||
         func  == KMSearchRule::FuncIsNotInCategory ) {
      const TQComboBox *combo=
        dynamic_cast<TQComboBox*>( QObject_child_const( valueStack,
                                                       "categoryCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( valueStack->child( "categoryCombo",
    //                                               0, false ) );
      if ( combo ) {
        return combo->currentText();
      }
      else {
        kdDebug(5006) << "TextRuleWidgetHandler::currentValue: "
                         "categoryCombo not found." << endl;
        return TQString::null;
      }
    }

    //in other cases of func it is a lineedit
    const RegExpLineEdit *lineEdit =
      dynamic_cast<RegExpLineEdit*>( QObject_child_const( valueStack,
                                                          "regExpLineEdit" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
    //                                                    0, false ) );
    if ( lineEdit ) {
      return lineEdit->text();
      }
    else
      kdDebug(5006) << "TextRuleWidgetHandler::currentValue: "
                       "regExpLineEdit not found." << endl;

    // or anything else, like addressbook
    return TQString::null;
  }

  //---------------------------------------------------------------------------

  TQString TextRuleWidgetHandler::value( const TQCString &,
                                        const TQWidgetStack *functionStack,
                                        const TQWidgetStack *valueStack ) const
  {
    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncIsInAddressbook )
      return "is in address book"; // just a non-empty dummy value
    else if ( func == KMSearchRule::FuncIsNotInAddressbook )
      return "is not in address book"; // just a non-empty dummy value
    else
      return currentValue( valueStack, func );
  }

  //---------------------------------------------------------------------------

  TQString TextRuleWidgetHandler::prettyValue( const TQCString &,
                                              const TQWidgetStack *functionStack,
                                              const TQWidgetStack *valueStack ) const
  {
    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncIsInAddressbook )
      return i18n( "is in address book" );
    else if ( func == KMSearchRule::FuncIsNotInAddressbook )
      return i18n( "is not in address book" );
    else
      return currentValue( valueStack, func );
  }

  //---------------------------------------------------------------------------

  bool TextRuleWidgetHandler::handlesField( const TQCString & ) const
  {
    return true; // we handle all fields (as fallback)
  }

  //---------------------------------------------------------------------------

  void TextRuleWidgetHandler::reset( TQWidgetStack *functionStack,
                                     TQWidgetStack *valueStack ) const
  {
    // reset the function combo box
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "textRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      funcCombo->setCurrentItem( 0 );
      funcCombo->blockSignals( false );
    }

    // reset the value widget
    RegExpLineEdit *lineEdit =
      dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                        0, false ) );
    if ( lineEdit ) {
      lineEdit->blockSignals( true );
      lineEdit->clear();
      lineEdit->blockSignals( false );
      lineEdit->showEditButton( false );
      valueStack->raiseWidget( lineEdit );
    }

    TQComboBox *combo =
      dynamic_cast<TQComboBox*>( valueStack->child( "categoryCombo",
                                                   0, false ) );
    if (combo) {
      combo->blockSignals( true );
      combo->setCurrentItem( 0 );
      combo->blockSignals( false );
    }
  }

  //---------------------------------------------------------------------------

  bool TextRuleWidgetHandler::setRule( TQWidgetStack *functionStack,
                                       TQWidgetStack *valueStack,
                                       const KMSearchRule *rule ) const
  {
    if ( !rule ) {
      reset( functionStack, valueStack );
      return false;
    }

    const KMSearchRule::Function func = rule->function();
    int i = 0;
    for ( ; i < TextFunctionCount; ++i )
      if ( func == TextFunctions[i].id )
        break;
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "textRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      if ( i < TextFunctionCount )
        funcCombo->setCurrentItem( i );
      else {
        kdDebug(5006) << "TextRuleWidgetHandler::setRule( "
                      << rule->asString()
                      << " ): unhandled function" << endl;
        funcCombo->setCurrentItem( 0 );
      }
      funcCombo->blockSignals( false );
      functionStack->raiseWidget( funcCombo );
    }

    if ( func == KMSearchRule::FuncIsInAddressbook ||
         func == KMSearchRule::FuncIsNotInAddressbook ) {
      TQWidget *w =
        static_cast<TQWidget*>( valueStack->child( "textRuleValueHider",
                                                  0, false ) );
      valueStack->raiseWidget( w );
    }
    else if ( func == KMSearchRule::FuncIsInCategory ||
              func == KMSearchRule::FuncIsNotInCategory) {
      TQComboBox *combo =
        static_cast<TQComboBox*>( valueStack->child( "categoryCombo",
                                                    0, false ) );
      combo->blockSignals( true );
      for ( i = 0; i < combo->count(); ++i )
        if ( rule->contents() == combo->text( i ) ) {
          combo->setCurrentItem( i );
          break;
        }
      if ( i == combo->count() )
        combo->setCurrentItem( 0 );

      combo->blockSignals( false );
      valueStack->raiseWidget( combo );
    }
    else {
      RegExpLineEdit *lineEdit =
        dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                          0, false ) );
      if ( lineEdit ) {
        lineEdit->blockSignals( true );
        lineEdit->setText( rule->contents() );
        lineEdit->blockSignals( false );
        lineEdit->showEditButton( func == KMSearchRule::FuncRegExp ||
                                  func == KMSearchRule::FuncNotRegExp );
        valueStack->raiseWidget( lineEdit );
      }
    }
    return true;
  }


  //---------------------------------------------------------------------------

  bool TextRuleWidgetHandler::update( const TQCString &,
                                      TQWidgetStack *functionStack,
                                      TQWidgetStack *valueStack ) const
  {
    // raise the correct function widget
    functionStack->raiseWidget(
      static_cast<TQWidget*>( functionStack->child( "textRuleFuncCombo",
                                                   0, false ) ) );

    // raise the correct value widget
    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncIsInAddressbook ||
         func == KMSearchRule::FuncIsNotInAddressbook ) {
      valueStack->raiseWidget(
        static_cast<TQWidget*>( valueStack->child( "textRuleValueHider",
                                                  0, false ) ) );
    }
    else if ( func == KMSearchRule::FuncIsInCategory ||
              func == KMSearchRule::FuncIsNotInCategory) {
      valueStack->raiseWidget(
        static_cast<TQWidget*>( valueStack->child( "categoryCombo",
                                                  0, false ) ) );
    }
    else {
      RegExpLineEdit *lineEdit =
        dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                          0, false ) );
      if ( lineEdit ) {
        lineEdit->showEditButton( func == KMSearchRule::FuncRegExp ||
                                  func == KMSearchRule::FuncNotRegExp );
        valueStack->raiseWidget( lineEdit );
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
  // also see KMSearchRule::matches() and KMSearchRule::Function
  // if you change the following strings!
  static const struct {
    const KMSearchRule::Function id;
    const char *displayName;
  } MessageFunctions[] = {
    { KMSearchRule::FuncContains,        I18N_NOOP( "contains" )          },
    { KMSearchRule::FuncContainsNot,     I18N_NOOP( "does not contain" )  },
    { KMSearchRule::FuncRegExp,          I18N_NOOP( "matches regular expr." ) },
    { KMSearchRule::FuncNotRegExp,       I18N_NOOP( "does not match reg. expr." ) },
    { KMSearchRule::FuncHasAttachment,   I18N_NOOP( "has an attachment" ) },
    { KMSearchRule::FuncHasNoAttachment, I18N_NOOP( "has no attachment" ) },
  };
  static const int MessageFunctionCount =
    sizeof( MessageFunctions ) / sizeof( *MessageFunctions );

  //---------------------------------------------------------------------------

  TQWidget * MessageRuleWidgetHandler::createFunctionWidget( int number,
                                                            TQWidgetStack *functionStack,
                                                            const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    TQComboBox *funcCombo = new TQComboBox( functionStack, "messageRuleFuncCombo" );
    for ( int i = 0; i < MessageFunctionCount; ++i ) {
      funcCombo->insertItem( i18n( MessageFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    TQObject::connect( funcCombo, TQT_SIGNAL( activated( int ) ),
                      receiver, TQT_SLOT( slotFunctionChanged() ) );
    return funcCombo;
  }

  //---------------------------------------------------------------------------

  TQWidget * MessageRuleWidgetHandler::createValueWidget( int number,
                                                         TQWidgetStack *valueStack,
                                                         const TQObject *receiver ) const
  {
    if ( number == 0 ) {
      RegExpLineEdit *lineEdit =
        new RegExpLineEdit( valueStack, "regExpLineEdit" );
      TQObject::connect( lineEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
                        receiver, TQT_SLOT( slotValueChanged() ) );
      return lineEdit;
    }

    // blank TQLabel to hide value widget for has-attachment rule
    if ( number == 1 ) {
      return new TQLabel( valueStack, "textRuleValueHider" );
    }

    return 0;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function MessageRuleWidgetHandler::currentFunction( const TQWidgetStack *functionStack ) const
  {
    const TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( QObject_child_const( functionStack,
                                                     "messageRuleFuncCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( functionStack->child( "messageRuleFuncCombo",
    //                                                  0, false ) );
    if ( funcCombo ) {
      return MessageFunctions[funcCombo->currentItem()].id;
    }
    else
      kdDebug(5006) << "MessageRuleWidgetHandler::currentFunction: "
                       "messageRuleFuncCombo not found." << endl;
    return KMSearchRule::FuncNone;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function MessageRuleWidgetHandler::function( const TQCString & field,
                                                             const TQWidgetStack *functionStack ) const
  {
    if ( !handlesField( field ) )
      return KMSearchRule::FuncNone;

    return currentFunction( functionStack );
  }

  //---------------------------------------------------------------------------

  TQString MessageRuleWidgetHandler::currentValue( const TQWidgetStack *valueStack,
                                                  KMSearchRule::Function ) const
  {
    const RegExpLineEdit *lineEdit =
      dynamic_cast<RegExpLineEdit*>( QObject_child_const( valueStack,
                                                          "regExpLineEdit" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
    //                                                    0, false ) );
    if ( lineEdit ) {
      return lineEdit->text();
    }
    else
      kdDebug(5006) << "MessageRuleWidgetHandler::currentValue: "
                       "regExpLineEdit not found." << endl;

    return TQString::null;
  }

  //---------------------------------------------------------------------------

  TQString MessageRuleWidgetHandler::value( const TQCString & field,
                                           const TQWidgetStack *functionStack,
                                           const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncHasAttachment )
      return "has an attachment"; // just a non-empty dummy value
    else if ( func == KMSearchRule::FuncHasNoAttachment )
      return "has no attachment"; // just a non-empty dummy value
    else
      return currentValue( valueStack, func );
  }

  //---------------------------------------------------------------------------

  TQString MessageRuleWidgetHandler::prettyValue( const TQCString & field,
                                                 const TQWidgetStack *functionStack,
                                                 const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncHasAttachment )
      return i18n( "has an attachment" );
    else if ( func == KMSearchRule::FuncHasNoAttachment )
      return i18n( "has no attachment" );
    else
      return currentValue( valueStack, func );
  }

  //---------------------------------------------------------------------------

  bool MessageRuleWidgetHandler::handlesField( const TQCString & field ) const
  {
    return ( field == "<message>" );
  }

  //---------------------------------------------------------------------------

  void MessageRuleWidgetHandler::reset( TQWidgetStack *functionStack,
                                        TQWidgetStack *valueStack ) const
  {
    // reset the function combo box
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "messageRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      funcCombo->setCurrentItem( 0 );
      funcCombo->blockSignals( false );
    }

    // reset the value widget
    RegExpLineEdit *lineEdit =
      dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                        0, false ) );
    if ( lineEdit ) {
      lineEdit->blockSignals( true );
      lineEdit->clear();
      lineEdit->blockSignals( false );
      lineEdit->showEditButton( false );
      valueStack->raiseWidget( lineEdit );
    }
  }

  //---------------------------------------------------------------------------

  bool MessageRuleWidgetHandler::setRule( TQWidgetStack *functionStack,
                                          TQWidgetStack *valueStack,
                                          const KMSearchRule *rule ) const
  {
    if ( !rule || !handlesField( rule->field() ) ) {
      reset( functionStack, valueStack );
      return false;
    }

    const KMSearchRule::Function func = rule->function();
    int i = 0;
    for ( ; i < MessageFunctionCount; ++i )
      if ( func == MessageFunctions[i].id )
        break;
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "messageRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      if ( i < MessageFunctionCount )
        funcCombo->setCurrentItem( i );
      else {
        kdDebug(5006) << "MessageRuleWidgetHandler::setRule( "
                      << rule->asString()
                      << " ): unhandled function" << endl;
        funcCombo->setCurrentItem( 0 );
      }
      funcCombo->blockSignals( false );
      functionStack->raiseWidget( funcCombo );
    }

    if ( func == KMSearchRule::FuncHasAttachment  ||
         func == KMSearchRule::FuncHasNoAttachment ) {
      TQWidget *w =
        static_cast<TQWidget*>( valueStack->child( "textRuleValueHider",
                                                  0, false ) );
      valueStack->raiseWidget( w );
    }
    else {
      RegExpLineEdit *lineEdit =
        dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                          0, false ) );
      if ( lineEdit ) {
        lineEdit->blockSignals( true );
        lineEdit->setText( rule->contents() );
        lineEdit->blockSignals( false );
        lineEdit->showEditButton( func == KMSearchRule::FuncRegExp ||
                                  func == KMSearchRule::FuncNotRegExp );
        valueStack->raiseWidget( lineEdit );
      }
    }
    return true;
  }


  //---------------------------------------------------------------------------

  bool MessageRuleWidgetHandler::update( const TQCString & field,
                                      TQWidgetStack *functionStack,
                                      TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return false;
    // raise the correct function widget
    functionStack->raiseWidget(
      static_cast<TQWidget*>( functionStack->child( "messageRuleFuncCombo",
                                                   0, false ) ) );

    // raise the correct value widget
    KMSearchRule::Function func = currentFunction( functionStack );
    if ( func == KMSearchRule::FuncHasAttachment  ||
         func == KMSearchRule::FuncHasNoAttachment ) {
      TQWidget *w =
        static_cast<TQWidget*>( valueStack->child( "textRuleValueHider",
                                                  0, false ) );
      valueStack->raiseWidget( w );
    }
    else {
      RegExpLineEdit *lineEdit =
        dynamic_cast<RegExpLineEdit*>( valueStack->child( "regExpLineEdit",
                                                          0, false ) );
      if ( lineEdit ) {
        lineEdit->showEditButton( func == KMSearchRule::FuncRegExp ||
                                  func == KMSearchRule::FuncNotRegExp );
        valueStack->raiseWidget( lineEdit );
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
    const KMSearchRule::Function id;
    const char *displayName;
  } StatusFunctions[] = {
    { KMSearchRule::FuncContains,    I18N_NOOP( "is" )    },
    { KMSearchRule::FuncContainsNot, I18N_NOOP( "is not" ) }
  };
  static const int StatusFunctionCount =
    sizeof( StatusFunctions ) / sizeof( *StatusFunctions );

  //---------------------------------------------------------------------------

  TQWidget * StatusRuleWidgetHandler::createFunctionWidget( int number,
                                                           TQWidgetStack *functionStack,
                                                           const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    TQComboBox *funcCombo = new TQComboBox( functionStack,
                                          "statusRuleFuncCombo" );
    for ( int i = 0; i < StatusFunctionCount; ++i ) {
      funcCombo->insertItem( i18n( StatusFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    TQObject::connect( funcCombo, TQT_SIGNAL( activated( int ) ),
                      receiver, TQT_SLOT( slotFunctionChanged() ) );
    return funcCombo;
  }

  //---------------------------------------------------------------------------

  TQWidget * StatusRuleWidgetHandler::createValueWidget( int number,
                                                        TQWidgetStack *valueStack,
                                                        const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    TQComboBox *statusCombo = new TQComboBox( valueStack,
                                            "statusRuleValueCombo" );
    for ( int i = 0; i < KMail::StatusValueCountWithoutHidden; ++i ) {
      statusCombo->insertItem( UserIcon( KMail::StatusValues[ i ].icon ), i18n( KMail::StatusValues[ i ].text ) );
    }
    statusCombo->adjustSize();
    TQObject::connect( statusCombo, TQT_SIGNAL( activated( int ) ),
                      receiver, TQT_SLOT( slotValueChanged() ) );
    return statusCombo;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function StatusRuleWidgetHandler::currentFunction( const TQWidgetStack *functionStack ) const
  {
    const TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( QObject_child_const( functionStack,
                                                     "statusRuleFuncCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( functionStack->child( "statusRuleFuncCombo",
    //                                                  0, false ) );
    if ( funcCombo ) {
      return StatusFunctions[funcCombo->currentItem()].id;
    }
    else
      kdDebug(5006) << "StatusRuleWidgetHandler::currentFunction: "
                       "statusRuleFuncCombo not found." << endl;
    return KMSearchRule::FuncNone;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function StatusRuleWidgetHandler::function( const TQCString & field,
                                                            const TQWidgetStack *functionStack ) const
  {
    if ( !handlesField( field ) )
      return KMSearchRule::FuncNone;

    return currentFunction( functionStack );
  }

  //---------------------------------------------------------------------------

  int StatusRuleWidgetHandler::currentStatusValue( const TQWidgetStack *valueStack ) const
  {
    const TQComboBox *statusCombo =
      dynamic_cast<TQComboBox*>( QObject_child_const( valueStack,
                                                     "statusRuleValueCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( valueStack->child( "statusRuleValueCombo",
    //                                               0, false ) );
    if ( statusCombo ) {
      return statusCombo->currentItem();
    }
    else
      kdDebug(5006) << "StatusRuleWidgetHandler::currentStatusValue: "
                       "statusRuleValueCombo not found." << endl;
    return -1;
  }

  //---------------------------------------------------------------------------

  TQString StatusRuleWidgetHandler::value( const TQCString & field,
                                          const TQWidgetStack *,
                                          const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    const int status = currentStatusValue( valueStack );
    if ( status != -1 )
      return TQString::fromLatin1( KMail::StatusValues[ status ].text );
    else
      return TQString::null;
  }

  //---------------------------------------------------------------------------

  TQString StatusRuleWidgetHandler::prettyValue( const TQCString & field,
                                                const TQWidgetStack *,
                                                const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    const int status = currentStatusValue( valueStack );
    if ( status != -1 )
      return i18n( KMail::StatusValues[ status ].text );
    else
      return TQString::null;
  }

  //---------------------------------------------------------------------------

  bool StatusRuleWidgetHandler::handlesField( const TQCString & field ) const
  {
    return ( field == "<status>" );
  }

  //---------------------------------------------------------------------------

  void StatusRuleWidgetHandler::reset( TQWidgetStack *functionStack,
                                       TQWidgetStack *valueStack ) const
  {
    // reset the function combo box
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "statusRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      funcCombo->setCurrentItem( 0 );
      funcCombo->blockSignals( false );
    }

    // reset the status value combo box
    TQComboBox *statusCombo =
      dynamic_cast<TQComboBox*>( valueStack->child( "statusRuleValueCombo",
                                                   0, false ) );
    if ( statusCombo ) {
      statusCombo->blockSignals( true );
      statusCombo->setCurrentItem( 0 );
      statusCombo->blockSignals( false );
    }
  }

  //---------------------------------------------------------------------------

  bool StatusRuleWidgetHandler::setRule( TQWidgetStack *functionStack,
                                         TQWidgetStack *valueStack,
                                         const KMSearchRule *rule ) const
  {
    if ( !rule || !handlesField( rule->field() ) ) {
      reset( functionStack, valueStack );
      return false;
    }

    // set the function
    const KMSearchRule::Function func = rule->function();
    int funcIndex = 0;
    for ( ; funcIndex < StatusFunctionCount; ++funcIndex )
      if ( func == StatusFunctions[funcIndex].id )
        break;
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "statusRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      if ( funcIndex < StatusFunctionCount )
        funcCombo->setCurrentItem( funcIndex );
      else {
        kdDebug(5006) << "StatusRuleWidgetHandler::setRule( "
                      << rule->asString()
                      << " ): unhandled function" << endl;
        funcCombo->setCurrentItem( 0 );
      }
      funcCombo->blockSignals( false );
      functionStack->raiseWidget( funcCombo );
    }

    // set the value
    const TQString value = rule->contents();
    int valueIndex = 0;
    for ( ; valueIndex < KMail::StatusValueCountWithoutHidden; ++valueIndex )
      if ( value == TQString::fromLatin1(
               KMail::StatusValues[ valueIndex ].text ) )
        break;
    TQComboBox *statusCombo =
      dynamic_cast<TQComboBox*>( valueStack->child( "statusRuleValueCombo",
                                                   0, false ) );
    if ( statusCombo ) {
      statusCombo->blockSignals( true );
      if ( valueIndex < KMail::StatusValueCountWithoutHidden )
        statusCombo->setCurrentItem( valueIndex );
      else {
        kdDebug(5006) << "StatusRuleWidgetHandler::setRule( "
                      << rule->asString()
                      << " ): unhandled value" << endl;
        statusCombo->setCurrentItem( 0 );
      }
      statusCombo->blockSignals( false );
      valueStack->raiseWidget( statusCombo );
    }
    return true;
  }


  //---------------------------------------------------------------------------

  bool StatusRuleWidgetHandler::update( const TQCString &field,
                                        TQWidgetStack *functionStack,
                                        TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return false;

    // raise the correct function widget
    functionStack->raiseWidget(
      static_cast<TQWidget*>( functionStack->child( "statusRuleFuncCombo",
                                                   0, false ) ) );

    // raise the correct value widget
    valueStack->raiseWidget(
      static_cast<TQWidget*>( valueStack->child( "statusRuleValueCombo",
                                                0, false ) ) );
    return true;
  }

} // anonymous namespace for StatusRuleWidgetHandler


//=============================================================================
//
// class NumericRuleWidgetHandler
//
//=============================================================================

namespace {
  static const struct {
    const KMSearchRule::Function id;
    const char *displayName;
  } NumericFunctions[] = {
    { KMSearchRule::FuncEquals,           I18N_NOOP( "is equal to" )         },
    { KMSearchRule::FuncNotEqual,         I18N_NOOP( "is not equal to" )      },
    { KMSearchRule::FuncIsGreater,        I18N_NOOP( "is greater than" )     },
    { KMSearchRule::FuncIsLessOrEqual,    I18N_NOOP( "is less than or equal to" ) },
    { KMSearchRule::FuncIsLess,           I18N_NOOP( "is less than" )        },
    { KMSearchRule::FuncIsGreaterOrEqual, I18N_NOOP( "is greater than or equal to" ) }
  };
  static const int NumericFunctionCount =
    sizeof( NumericFunctions ) / sizeof( *NumericFunctions );

  //---------------------------------------------------------------------------

  TQWidget * NumericRuleWidgetHandler::createFunctionWidget( int number,
                                                            TQWidgetStack *functionStack,
                                                            const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    TQComboBox *funcCombo = new TQComboBox( functionStack,
                                          "numericRuleFuncCombo" );
    for ( int i = 0; i < NumericFunctionCount; ++i ) {
      funcCombo->insertItem( i18n( NumericFunctions[i].displayName ) );
    }
    funcCombo->adjustSize();
    TQObject::connect( funcCombo, TQT_SIGNAL( activated( int ) ),
                      receiver, TQT_SLOT( slotFunctionChanged() ) );
    return funcCombo;
  }

  //---------------------------------------------------------------------------

  TQWidget * NumericRuleWidgetHandler::createValueWidget( int number,
                                                         TQWidgetStack *valueStack,
                                                         const TQObject *receiver ) const
  {
    if ( number != 0 )
      return 0;

    KIntNumInput *numInput = new KIntNumInput( valueStack, "KIntNumInput" );
    TQObject::connect( numInput, TQT_SIGNAL( valueChanged( int ) ),
                      receiver, TQT_SLOT( slotValueChanged() ) );
    return numInput;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function NumericRuleWidgetHandler::currentFunction( const TQWidgetStack *functionStack ) const
  {
    const TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( QObject_child_const( functionStack,
                                                     "numericRuleFuncCombo" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<TQComboBox*>( functionStack->child( "numericRuleFuncCombo",
    //                                                  0, false ) );
    if ( funcCombo ) {
      return NumericFunctions[funcCombo->currentItem()].id;
    }
    else
      kdDebug(5006) << "NumericRuleWidgetHandler::currentFunction: "
                       "numericRuleFuncCombo not found." << endl;
    return KMSearchRule::FuncNone;
  }

  //---------------------------------------------------------------------------

  KMSearchRule::Function NumericRuleWidgetHandler::function( const TQCString & field,
                                                             const TQWidgetStack *functionStack ) const
  {
    if ( !handlesField( field ) )
      return KMSearchRule::FuncNone;

    return currentFunction( functionStack );
  }

  //---------------------------------------------------------------------------

  TQString NumericRuleWidgetHandler::currentValue( const TQWidgetStack *valueStack ) const
  {
    const KIntNumInput *numInput =
      dynamic_cast<KIntNumInput*>( QObject_child_const( valueStack,
                                                        "KIntNumInput" ) );
    // FIXME (Qt >= 4.0): Use the following when TQObject::child() is const.
    //  dynamic_cast<KIntNumInput*>( valueStack->child( "KIntNumInput",
    //                                                  0, false ) );
    if ( numInput ) {
      return TQString::number( numInput->value() );
    }
    else
      kdDebug(5006) << "NumericRuleWidgetHandler::currentValue: "
                       "KIntNumInput not found." << endl;
    return TQString::null;
  }

  //---------------------------------------------------------------------------

  TQString NumericRuleWidgetHandler::value( const TQCString & field,
                                           const TQWidgetStack *,
                                           const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    return currentValue( valueStack );
  }

  //---------------------------------------------------------------------------

  TQString NumericRuleWidgetHandler::prettyValue( const TQCString & field,
                                                 const TQWidgetStack *,
                                                 const TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return TQString::null;

    return currentValue( valueStack );
  }

  //---------------------------------------------------------------------------

  bool NumericRuleWidgetHandler::handlesField( const TQCString & field ) const
  {
    return ( field == "<size>" || field == "<age in days>" );
  }

  //---------------------------------------------------------------------------

  void NumericRuleWidgetHandler::reset( TQWidgetStack *functionStack,
                                        TQWidgetStack *valueStack ) const
  {
    // reset the function combo box
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "numericRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      funcCombo->setCurrentItem( 0 );
      funcCombo->blockSignals( false );
    }

    // reset the value widget
    KIntNumInput *numInput =
      dynamic_cast<KIntNumInput*>( valueStack->child( "KIntNumInput",
                                                      0, false ) );
    if ( numInput ) {
      numInput->blockSignals( true );
      numInput->setValue( 0 );
      numInput->blockSignals( false );
    }
  }

  //---------------------------------------------------------------------------

  void initNumInput( KIntNumInput *numInput, const TQCString &field )
  {
    if ( field == "<size>" ) {
      numInput->setMinValue( 0 );
      numInput->setSuffix( i18n( " bytes" ) );
    }
    else {
      numInput->setMinValue( -10000 );
      numInput->setSuffix( i18n( " days" ) );
    }
  }

  //---------------------------------------------------------------------------

  bool NumericRuleWidgetHandler::setRule( TQWidgetStack *functionStack,
                                          TQWidgetStack *valueStack,
                                          const KMSearchRule *rule ) const
  {
    if ( !rule || !handlesField( rule->field() ) ) {
      reset( functionStack, valueStack );
      return false;
    }

    // set the function
    const KMSearchRule::Function func = rule->function();
    int funcIndex = 0;
    for ( ; funcIndex < NumericFunctionCount; ++funcIndex )
      if ( func == NumericFunctions[funcIndex].id )
        break;
    TQComboBox *funcCombo =
      dynamic_cast<TQComboBox*>( functionStack->child( "numericRuleFuncCombo",
                                                      0, false ) );
    if ( funcCombo ) {
      funcCombo->blockSignals( true );
      if ( funcIndex < NumericFunctionCount )
        funcCombo->setCurrentItem( funcIndex );
      else {
        kdDebug(5006) << "NumericRuleWidgetHandler::setRule( "
                      << rule->asString()
                      << " ): unhandled function" << endl;
        funcCombo->setCurrentItem( 0 );
      }
      funcCombo->blockSignals( false );
      functionStack->raiseWidget( funcCombo );
    }

    // set the value
    bool ok;
    int value = rule->contents().toInt( &ok );
    if ( !ok )
      value = 0;
    KIntNumInput *numInput =
      dynamic_cast<KIntNumInput*>( valueStack->child( "KIntNumInput",
                                                      0, false ) );
    if ( numInput ) {
      initNumInput( numInput, rule->field() );
      numInput->blockSignals( true );
      numInput->setValue( value );
      numInput->blockSignals( false );
      valueStack->raiseWidget( numInput );
    }
    return true;
  }


  //---------------------------------------------------------------------------

  bool NumericRuleWidgetHandler::update( const TQCString &field,
                                         TQWidgetStack *functionStack,
                                         TQWidgetStack *valueStack ) const
  {
    if ( !handlesField( field ) )
      return false;

    // raise the correct function widget
    functionStack->raiseWidget(
      static_cast<TQWidget*>( functionStack->child( "numericRuleFuncCombo",
                                                   0, false ) ) );

    // raise the correct value widget
    KIntNumInput *numInput =
      dynamic_cast<KIntNumInput*>( valueStack->child( "KIntNumInput",
                                                      0, false ) );
    if ( numInput ) {
      initNumInput( numInput, field );
      valueStack->raiseWidget( numInput );
    }
    return true;
  }

} // anonymous namespace for NumericRuleWidgetHandler

