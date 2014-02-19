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

#ifndef MAILCOMMON_INTERFACES_RULEWIDGETHANDLER_H
#define MAILCOMMON_INTERFACES_RULEWIDGETHANDLER_H

#include "searchpattern.h"

#include <QByteArray>

class QWidget;
class QStackedWidget;
class QByteArray;
class QString;
class QObject;

namespace MailCommon {

/**
 * @short An interface to filter/search rule widget handlers
 */
class RuleWidgetHandler
{
  public:
    virtual ~RuleWidgetHandler() {}

    virtual QWidget * createFunctionWidget( int number,
                                            QStackedWidget *functionStack,
                                            const QObject *receiver, bool isBalooSearch ) const = 0;
    virtual QWidget * createValueWidget( int number,
                                         QStackedWidget *valueStack,
                                         const QObject *receiver ) const = 0;
    virtual MailCommon::SearchRule::Function function( const QByteArray & field,
                                             const QStackedWidget *functionStack ) const = 0;
    virtual QString value( const QByteArray & field,
                           const QStackedWidget *functionStack,
                           const QStackedWidget *valueStack ) const = 0;
    virtual QString prettyValue( const QByteArray & field,
                                 const QStackedWidget *functionStack,
                                 const QStackedWidget *valueStack ) const = 0;
    virtual bool handlesField( const QByteArray & field ) const = 0;
    virtual void reset( QStackedWidget *functionStack,
                        QStackedWidget *valueStack ) const = 0;
    virtual bool setRule( QStackedWidget *functionStack,
                          QStackedWidget *valueStack,
                          const MailCommon::SearchRule::Ptr rule, bool isBalooSearch ) const = 0;
    virtual bool update( const QByteArray & field,
                         QStackedWidget *functionStack,
                         QStackedWidget *valueStack ) const = 0;

};

}

#endif

