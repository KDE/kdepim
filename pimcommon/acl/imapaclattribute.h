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

#ifndef MAILCOMMON_IMAPACLATTRIBUTE_H
#define MAILCOMMON_IMAPACLATTRIBUTE_H

#include "pimcommon_export.h"

#include <Akonadi/Attribute>

#include <KIMAP/Acl>

#include <QMap>

namespace PimCommon {

class PIMCOMMON_EXPORT ImapAclAttribute : public Akonadi::Attribute
{
public:
    ImapAclAttribute();
    ImapAclAttribute( const QMap<QByteArray, KIMAP::Acl::Rights> &rights,
                      const QMap<QByteArray, KIMAP::Acl::Rights> &oldRights );
    void setRights( const QMap<QByteArray, KIMAP::Acl::Rights> &rights );
    QMap<QByteArray, KIMAP::Acl::Rights> rights() const;
    QMap<QByteArray, KIMAP::Acl::Rights> oldRights() const;
    QByteArray type() const;
    Attribute *clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray &data );

private:
    QMap<QByteArray, KIMAP::Acl::Rights> mRights;
    QMap<QByteArray, KIMAP::Acl::Rights> mOldRights;
};

}

#endif
