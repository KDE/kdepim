/*
    This file is part of Akregator2.

    Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SEARCHDESCRIPTIONATTRIBUTE_H
#define SEARCHDESCRIPTIONATTRIBUTE_H

#include <Akonadi/Attribute>
#include <Akonadi/Collection>

#include <QByteArray>

namespace Akregator2 {

class SearchDescriptionAttribute : public Akonadi::Attribute
{
  public:
    SearchDescriptionAttribute();
    virtual QByteArray type() const;
    virtual Attribute* clone() const;
    virtual QByteArray serialized() const;
    virtual void deserialize( const QByteArray& data );

    Akonadi::Collection baseCollection() const;
    void setBaseCollection( const Akonadi::Collection& collection );

    bool recursive() const;
    void setRecursive( bool recursive );

    QVariant searchPattern() const;
    void setSearchPattern( const QVariant& searchPattern );

    QString description() const;
    void setDescription( const QString &description );

  private:
    Akonadi::Collection mBaseCollection;
    bool mRecursive;
    QVariant mSearchPattern;
    QString mDescription;

};

} /* namespace Akregator2 */

#endif // SEARCHDESCRIPTIONATTRIBUTE_H
