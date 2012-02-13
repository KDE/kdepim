/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef FILTERACTIONSTATUS_H
#define FILTERACTIONSTATUS_H

#include "filteractionwithstringlist.h"
#include <KDE/Akonadi/KMime/MessageStatus>

namespace MailCommon {

class FilterActionStatus : public FilterActionWithStringList
{
public:
    FilterActionStatus(const char *name, const QString &label, QObject *parent = 0);
    virtual bool requiresBody() const;
    virtual bool isEmpty() const;
    virtual void argsFromString( const QString &argsStr );
    virtual QString argsAsString() const;
    virtual QString displayString() const;

    static QString realStatusString( const QString &statusStr );

    static Akonadi::MessageStatus stati[];
    static int StatiCount;
};

}
#endif // FILTERACTIONSTATUS_H
