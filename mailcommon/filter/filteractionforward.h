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

#ifndef MAILCOMMON_FILTERACTIONFORWARD_H
#define MAILCOMMON_FILTERACTIONFORWARD_H

#include "filteractionwithaddress.h"

namespace MailCommon
{

//=============================================================================
// FilterActionForward - forward to
// Forward message to another user, with a defined template
//=============================================================================
class FilterActionForward: public FilterActionWithAddress
{
    Q_OBJECT
public:
    explicit FilterActionForward(QObject *parent = 0);
    static FilterAction *newAction();
    ReturnCode process(ItemContext &context, bool applyOnOutbound) const;
    SearchRule::RequiredPart requiredPart() const;
    QWidget *createParamWidget(QWidget *parent) const;
    void applyParamWidgetValue(QWidget *paramWidget);
    void setParamWidgetValue(QWidget *paramWidget) const;
    void clearParamWidget(QWidget *paramWidget) const;
    void argsFromString(const QString &argsStr);
    QString argsAsString() const;
    QString displayString() const;
    bool argsFromStringInteractive(const QString &argsStr, const QString &filterName);

private:
    mutable QString mTemplate;
};

}

#endif
