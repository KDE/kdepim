/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef INVALIDFILTERINFO_H
#define INVALIDFILTERINFO_H

#include <QString>
#include "mailcommon_export.h"
namespace MailCommon
{
class MAILCOMMON_EXPORT InvalidFilterInfo
{
public:
    InvalidFilterInfo();
    InvalidFilterInfo(const QString &name, const QString &information);

    QString information() const;

    QString name() const;

    void setName(const QString &name);

    void setInformation(const QString &information);

    bool operator ==(const InvalidFilterInfo &other) const;
private:
    QString mName;
    QString mInformation;
};
}
Q_DECLARE_TYPEINFO(MailCommon::InvalidFilterInfo, Q_MOVABLE_TYPE);

#endif // INVALIDFILTERINFO_H
