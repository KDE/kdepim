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

#ifndef MAILCOMMON_FILTERACTIONWITHNONE_H
#define MAILCOMMON_FILTERACTIONWITHNONE_H

#include "filteraction.h"

namespace MailCommon
{

/**
 *  @short Abstract base class for filter actions with no parameter.
 *
 *  Abstract base class for mail filter actions that need no
 *  parameter, e.g. "Confirm Delivery". Creates an (empty) QWidget as
 *  parameter widget. A subclass of this must provide at least
 *  implementations for the following methods:
 *
 *  @li virtual FilterAction::ReturnCodes FilterAction::process
 *  @li static FilterAction::newAction
 *
 *  @author Marc Mutz <mutz@kde.org>, based upon work by Stefan Taferner <taferner@kde.org>
 *  @see FilterAction Filter
 */
class FilterActionWithNone : public FilterAction
{
    Q_OBJECT

public:
    /**
     * @copydoc FilterAction::FilterAction
     */
    FilterActionWithNone(const QString &name, const QString &label, QObject *parent = 0);

    /**
     * @copydoc FilterAction::argsFromString
     */
    virtual void argsFromString(const QString &);

    /**
     * @copydoc FilterAction::argsAsString
     */
    virtual QString argsAsString() const;

    /**
     * @copydoc FilterAction::displayString
     */
    virtual QString displayString() const;
};

}

#endif
