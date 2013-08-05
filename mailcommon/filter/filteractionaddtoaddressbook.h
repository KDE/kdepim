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

namespace MailCommon {

//=============================================================================
// FilterActionAddToAddressBook
// - add email address from header to address book
//=============================================================================
class FilterActionAddToAddressBook: public FilterActionWithStringList
{
    Q_OBJECT
public:
    explicit FilterActionAddToAddressBook( QObject *parent = 0 );
    ReturnCode process( ItemContext &context ) const;
    static FilterAction* newAction();

    SearchRule::RequiredPart requiredPart() const;

    bool isEmpty() const;

    QWidget* createParamWidget( QWidget *parent ) const;
    void setParamWidgetValue( QWidget *paramWidget ) const;
    void applyParamWidgetValue( QWidget *paramWidget );
    void clearParamWidget( QWidget *paramWidget ) const;

    QString argsAsString() const;
    void argsFromString( const QString &argsStr );

private:
    enum HeaderType
    {
        FromHeader,
        ToHeader,
        CcHeader,
        BccHeader
    };

    const QString mFromStr, mToStr, mCCStr, mBCCStr;
    HeaderType mHeaderType;
    Akonadi::Collection::Id mCollectionId;
    QString mCategory;
};

}

#endif
