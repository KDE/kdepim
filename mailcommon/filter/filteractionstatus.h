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
#include <KDE/KMime/MessageStatus>

namespace MailCommon {

class FilterActionStatus : public FilterActionWithStringList
{
    Q_OBJECT
public:
    FilterActionStatus(const QString &name, const QString &label, QObject *parent = 0);
    SearchRule::RequiredPart requiredPart() const;
    bool isEmpty() const;
    void argsFromString( const QString &argsStr );
    QString argsAsString() const;
    QString displayString() const;

    static QString realStatusString( const QString &statusStr );

    static Akonadi::MessageStatus stati[];
    static int StatiCount;
};

}
#endif // FILTERACTIONSTATUS_H
