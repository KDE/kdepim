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

namespace MailCommon
{

//=============================================================================
// FilterActionSetTransport - set transport to...
// Specify mail transport (smtp server) to be used when replying to a message
//=============================================================================
class FilterActionSetTransport: public FilterAction
{
    Q_OBJECT
public:
    explicit FilterActionSetTransport(QObject *parent = Q_NULLPTR);
    ReturnCode process(ItemContext &context, bool applyOnOutbound) const Q_DECL_OVERRIDE;
    SearchRule::RequiredPart requiredPart() const Q_DECL_OVERRIDE;
    static FilterAction *newAction();
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

    bool argsFromStringInteractive(const QString &argsStr, const QString &filterName) Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::argsFromString
     */
    void argsFromString(const QString &argsStr) Q_DECL_OVERRIDE;
    /**
     * @copydoc FilterAction::isEmpty
     */
    bool isEmpty() const Q_DECL_OVERRIDE;
    /**
     * @copydoc FilterAction::argsAsString
     */
    QString argsAsString() const Q_DECL_OVERRIDE;

    /**
     * @copydoc FilterAction::displayString
     */
    QString displayString() const Q_DECL_OVERRIDE;
    QString informationAboutNotValidAction() const Q_DECL_OVERRIDE;

protected:
    int mParameter;
    mutable QString mTransportName;
};

}

#endif
