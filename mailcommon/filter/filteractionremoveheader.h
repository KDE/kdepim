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

#ifndef MAILCOMMON_FILTERACTIONREMOVEHEADER_H
#define MAILCOMMON_FILTERACTIONREMOVEHEADER_H

#include "filteractionwithstringlist.h"

namespace MailCommon {

//=============================================================================
// FilterActionRemoveHeader - remove header
// Remove all instances of the given header field.
//=============================================================================
class FilterActionRemoveHeader: public FilterActionWithStringList
{
    Q_OBJECT
public:
    explicit FilterActionRemoveHeader( QObject *parent = 0 );
    ReturnCode process( ItemContext &context ) const;
    SearchRule::RequiredPart requiredPart() const;
    QWidget* createParamWidget( QWidget *parent ) const;
    void setParamWidgetValue( QWidget *paramWidget ) const;

    bool canConvertToSieve() const;
    QStringList sieveRequires() const;
    QString sieveCode() const;

    static FilterAction* newAction();
};

}

#endif
