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

#ifndef MAILCOMMON_FILTERACTIONWITHURL_H
#define MAILCOMMON_FILTERACTIONWITHURL_H

#include "filteraction.h"

namespace MailCommon {

/**
 * @short Abstract base class for filter actions with a command line as parameter.
 *
 * Abstract base class for mail filter actions that need a command
 * line as parameter, e.g. 'forward to'. Can create a QLineEdit
 * (are there better widgets in the depths of the kdelibs?) as
 * parameter widget. A subclass of this must provide at least
 * implementations for the following methods:
 *
 * @li virtual FilterAction::ReturnCodes FilterAction::process
 * @li static FilterAction::newAction
 *
 * The implementation of FilterAction::process should take the
 * command line specified in mParameter, make all required
 * modifications and stream the resulting command line into @p
 * mProcess. Then you can start the command with @p mProcess.start().
 *
 * @author Marc Mutz <mutz@kde.org>, based upon work by Stefan Taferner <taferner@kde.org>
 * @see FilterActionWithString FilterAction Filter KProcess
 */
class FilterActionWithUrl : public FilterAction
{
  Q_OBJECT
  public:
    /**
     * @copydoc FilterAction::FilterAction
     */
    FilterActionWithUrl(const QString &name, const QString &label, QObject *parent = 0 );

    /**
     * @copydoc FilterAction::~FilterAction
     */
    ~FilterActionWithUrl();

    /**
     * @copydoc FilterAction::isEmpty
     */
    virtual bool isEmpty() const;

    /**
     * @copydoc FilterAction::createParamWidget
     */
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

    /**
     * @copydoc FilterAction::applyFromString
     */
    virtual void argsFromString( const QString &argsStr );

    /**
     * @copydoc FilterAction::argsAsString
     */
    virtual QString argsAsString() const;

    /**
     * @copydoc FilterAction::displayString
     */
    virtual QString displayString() const;

  protected:
    QString mParameter;
};

}

#endif
