/*
  Copyright (C) 2008 Omat Holding B.V. <info@omat.nl>

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

#ifndef MAILCOMMON_COLLECTIONANNOTATIONSATTRIBUTE_H
#define MAILCOMMON_COLLECTIONANNOTATIONSATTRIBUTE_H

#include <Attribute>
#include "mailcommon_export.h"

#include <QMap>

namespace MailCommon
{

class MAILCOMMON_EXPORT CollectionAnnotationsAttribute : public Akonadi::Attribute
{
public:
    CollectionAnnotationsAttribute();
    CollectionAnnotationsAttribute(const QMap<QByteArray, QByteArray> &annotations);

    void setAnnotations(const QMap<QByteArray, QByteArray> &annotations);
    QMap<QByteArray, QByteArray> annotations() const;

    QByteArray type() const Q_DECL_OVERRIDE;
    CollectionAnnotationsAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

    bool operator==(const CollectionAnnotationsAttribute &other) const;

private:
    QMap<QByteArray, QByteArray> mAnnotations;
};

}

#endif
