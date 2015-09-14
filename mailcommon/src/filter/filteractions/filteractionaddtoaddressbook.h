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

#ifndef MAILCOMMON_FILTERACTIONADDTOADDRESSBOOK_H
#define MAILCOMMON_FILTERACTIONADDTOADDRESSBOOK_H

#include "filteractionwithstringlist.h"

namespace MailCommon
{

//=============================================================================
// FilterActionAddToAddressBook
// - add email address from header to address book
//=============================================================================
class FilterActionAddToAddressBook: public FilterActionWithStringList
{
    Q_OBJECT
public:
    explicit FilterActionAddToAddressBook(QObject *parent = Q_NULLPTR);
    ReturnCode process(ItemContext &context, bool applyOnOutbound) const Q_DECL_OVERRIDE;
    static FilterAction *newAction();

    SearchRule::RequiredPart requiredPart() const Q_DECL_OVERRIDE;

    bool isEmpty() const Q_DECL_OVERRIDE;

    QWidget *createParamWidget(QWidget *parent) const Q_DECL_OVERRIDE;
    void setParamWidgetValue(QWidget *paramWidget) const Q_DECL_OVERRIDE;
    void applyParamWidgetValue(QWidget *paramWidget) Q_DECL_OVERRIDE;
    void clearParamWidget(QWidget *paramWidget) const Q_DECL_OVERRIDE;

    QString argsAsString() const Q_DECL_OVERRIDE;
    void argsFromString(const QString &argsStr) Q_DECL_OVERRIDE;

    QString informationAboutNotValidAction() const Q_DECL_OVERRIDE;
private:
    enum HeaderType {
        FromHeader,
        ToHeader,
        CcHeader,
        BccHeader,
        UnknownHeader
    };

    const QString mFromStr, mToStr, mCCStr, mBCCStr;
    HeaderType mHeaderType;
    Akonadi::Collection::Id mCollectionId;
    QString mCategory;
};

}

#endif
