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

#ifndef MAILCOMMON_FILTERACTIONREWRITEHEADER_H
#define MAILCOMMON_FILTERACTIONREWRITEHEADER_H

#include "filteractionwithstringlist.h"

#include <QtCore/QRegExp>

namespace MailCommon {

//=============================================================================
// FilterActionRewriteHeader - rewrite header
// Rewrite a header using a regexp.
//=============================================================================
class FilterActionRewriteHeader: public FilterActionWithStringList
{
  Q_OBJECT
  public:
    explicit FilterActionRewriteHeader( QObject *parent = 0 );
    virtual ReturnCode process( ItemContext &context ) const;
    virtual SearchRule::RequiredPart requiredPart() const;
    virtual QWidget* createParamWidget( QWidget *parent ) const;
    virtual void setParamWidgetValue( QWidget *paramWidget ) const;
    virtual void applyParamWidgetValue( QWidget *paramWidget );
    virtual void clearParamWidget( QWidget *paramWidget ) const;

    virtual QString argsAsString() const;
    virtual void argsFromString( const QString &argsStr );

    virtual QString displayString() const;

    static FilterAction* newAction();

  private:
    QRegExp mRegExp;
    QString mReplacementString;
};

}

#endif
