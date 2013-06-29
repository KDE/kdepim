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

#include <Akonadi/Attribute>

#include <QMap>

namespace MailCommon {

class CollectionAnnotationsAttribute : public Akonadi::Attribute
{
public:
    CollectionAnnotationsAttribute();
    CollectionAnnotationsAttribute( const QMap<QByteArray, QByteArray> &annotations );

    void setAnnotations( const QMap<QByteArray, QByteArray> &annotations );
    QMap<QByteArray, QByteArray> annotations() const;

    QByteArray type() const;
    Attribute *clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray &data );

private:
    QMap<QByteArray, QByteArray> mAnnotations;
};

}

#endif
