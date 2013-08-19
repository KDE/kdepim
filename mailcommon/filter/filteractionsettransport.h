/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MAILCOMMON_FILTERACTIONSETTRANSPORT_H
#define MAILCOMMON_FILTERACTIONSETTRANSPORT_H

#include "filteraction.h"

namespace MailCommon {

//=============================================================================
// FilterActionSetTransport - set transport to...
// Specify mail transport (smtp server) to be used when replying to a message
//=============================================================================
class FilterActionSetTransport: public FilterAction
{
  Q_OBJECT
  public:
    explicit FilterActionSetTransport( QObject *parent = 0 );
    virtual ReturnCode process( ItemContext &context ) const;
    virtual SearchRule::RequiredPart requiredPart() const;
    static FilterAction *newAction();
    virtual QWidget *createParamWidget( QWidget *parent ) const;
    /**
     * @copydoc FilterAction::applyParamWidgetValue
     */
    virtual void applyParamWidgetValue( QWidget *paramWidget );

    /**
     * @copydoc FilterAction::setParamWidgetValue
     */
    virtual void setParamWidgetValue( QWidget *paramWidget ) const;

    /**
     * @copydoc FilterAction::clearParamWidget
     */
    virtual void clearParamWidget( QWidget *paramWidget ) const;

    virtual bool argsFromStringInteractive( const QString &argsStr, const QString &filterName );

    /**
     * @copydoc FilterAction::argsFromString
     */
    virtual void argsFromString( const QString &argsStr );
    /**
     * @copydoc FilterAction::isEmpty
     */
    virtual bool isEmpty() const;
    /**
     * @copydoc FilterAction::argsAsString
     */
    virtual QString argsAsString() const;

    /**
     * @copydoc FilterAction::displayString
     */
    virtual QString displayString() const;

  protected:
    int mParameter;
    mutable QString mTransportName;
};

}

#endif
