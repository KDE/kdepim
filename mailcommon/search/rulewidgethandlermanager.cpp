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
#include "textrulerwidgethandler.h"
#include "statusrulewidgethandler.h"
#include "messagerulewidgethandler.h"
#include "numericrulewidgethandler.h"
#include "tagrulewidgethandler.h"
#include "daterulewidgethandler.h"
#include "numericdoublerulewidgethandler.h"
#include "headersrulerwidgethandler.h"
#include "interfaces/rulewidgethandler.h"

#include <messageviewer/viewer/stl_util.h>

#include <KDebug>
#include <QObject>
#include <QStackedWidget>

#include "search/searchpattern.h"

#include <KLocale>


#include <algorithm>
using std::for_each;
using std::remove;

using namespace MailCommon;

MailCommon::RuleWidgetHandlerManager *MailCommon::RuleWidgetHandlerManager::self = 0;

MailCommon::RuleWidgetHandlerManager::RuleWidgetHandlerManager()
    : mIsNepomukSearch(false)
{
    registerHandler( new MailCommon::TagRuleWidgetHandler() );
    registerHandler( new MailCommon::DateRuleWidgetHandler() );
    registerHandler( new MailCommon::NumericRuleWidgetHandler() );
    registerHandler( new MailCommon::StatusRuleWidgetHandler() );
    registerHandler( new MailCommon::MessageRuleWidgetHandler() );
    registerHandler( new MailCommon::NumericDoubleRuleWidgetHandler() );
    registerHandler( new MailCommon::HeadersRuleWidgetHandler() );
    // the TextRuleWidgetHandler is the fallback handler, so it has to be added
    // as last handler
    registerHandler( new MailCommon::TextRuleWidgetHandler() );
}

MailCommon::RuleWidgetHandlerManager::~RuleWidgetHandlerManager()
{
    for_each( mHandlers.begin(), mHandlers.end(),
              MessageViewer::DeleteAndSetToZero<RuleWidgetHandler>() );
}

void MailCommon::RuleWidgetHandlerManager::setIsNepomukSearch(bool isNepomukSearch)
{
    mIsNepomukSearch = isNepomukSearch;
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

void MailCommon::RuleWidgetHandlerManager::createWidgets(QStackedWidget *functionStack,
                                                          QStackedWidget *valueStack,
                                                          const QObject *receiver) const
{
    const_iterator end( mHandlers.constEnd() );
    for ( const_iterator it = mHandlers.constBegin(); it != end; ++it ) {
        QWidget *w = 0;
        for ( int i = 0;
              ( w = (*it)->createFunctionWidget( i, functionStack, receiver, mIsNepomukSearch ) );
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
        if ( (*it)->setRule( functionStack, valueStack, rule, mIsNepomukSearch ) ) {
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
