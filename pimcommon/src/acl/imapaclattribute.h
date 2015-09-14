/*
  Copyright (C) 2009 Kevin Ottens <ervin@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

// THIS file should not exist and is only a copy of
// kdepim-runtime/resources/shared/singlefileresource

// Any improvements should be done at kdepim-runtime and
// than afterwards copy the new version

#ifndef MAILCOMMON_IMAPACLATTRIBUTE_H
#define MAILCOMMON_IMAPACLATTRIBUTE_H

#include "pimcommon_export.h"

#include <Attribute>

#include <KIMAP/Acl>

#include <QMap>

namespace PimCommon
{
class ImapAclAttributePrivate;
class PIMCOMMON_EXPORT ImapAclAttribute : public Akonadi::Attribute
{
public:
    ImapAclAttribute();
    ImapAclAttribute(const QMap<QByteArray, KIMAP::Acl::Rights> &rights,
                     const QMap<QByteArray, KIMAP::Acl::Rights> &oldRights);
    ~ImapAclAttribute();
    void setRights(const QMap<QByteArray, KIMAP::Acl::Rights> &rights);
    QMap<QByteArray, KIMAP::Acl::Rights> rights() const;
    QMap<QByteArray, KIMAP::Acl::Rights> oldRights() const;
    void setMyRights(KIMAP::Acl::Rights rights);
    KIMAP::Acl::Rights myRights() const;
    QByteArray type() const Q_DECL_OVERRIDE;
    ImapAclAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

    bool operator==(const ImapAclAttribute &other) const;
private:
    ImapAclAttributePrivate *const d;
    QMap<QByteArray, KIMAP::Acl::Rights> mRights;
    QMap<QByteArray, KIMAP::Acl::Rights> mOldRights;
    KIMAP::Acl::Rights mMyRights;
};

}

#endif
