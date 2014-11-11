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

namespace MailCommon
{

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
    FilterActionWithUrl(const QString &name, const QString &label, QObject *parent = 0);

    /**
     * @copydoc FilterAction::~FilterAction
     */
    ~FilterActionWithUrl();

    /**
     * @copydoc FilterAction::isEmpty
     */
    bool isEmpty() const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::createParamWidget
     */
    QWidget *createParamWidget(QWidget *parent) const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::applyParamWidgetValue
     */
    void applyParamWidgetValue(QWidget *paramWidget) Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::setParamWidgetValue
     */
    void setParamWidgetValue(QWidget *paramWidget) const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::clearParamWidget
     */
    void clearParamWidget(QWidget *paramWidget) const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::applyFromString
     */
    void argsFromString(const QString &argsStr) Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::argsAsString
     */
    QString argsAsString() const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::displayString
     */
    QString displayString() const Q_DECL_OVERRIDE;

protected:
    QString mParameter;
};

}

#endif
