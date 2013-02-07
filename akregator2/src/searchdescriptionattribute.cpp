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

#include "searchdescriptionattribute.h"

namespace Akregator2 {

SearchDescriptionAttribute::SearchDescriptionAttribute():
  mRecursive(false)
{
}

QByteArray SearchDescriptionAttribute::type() const
{
  return "akregatorsearchdescription";
}

Akonadi::Attribute* SearchDescriptionAttribute::clone() const
{
  return new SearchDescriptionAttribute( *this );
}

QByteArray SearchDescriptionAttribute::serialized() const
{
  QByteArray ba;
  QDataStream s( &ba, QIODevice::WriteOnly );
  s << mBaseCollection.id();
  s << mRecursive;
  s << mDescription;
  s << mSearchPattern;

  return ba;
}

void SearchDescriptionAttribute::deserialize(const QByteArray& data)
{
  QDataStream s( data );
  Akonadi::Entity::Id id;
  s >> id;
  mBaseCollection = Akonadi::Collection( id );
  s >> mRecursive;
  s >> mDescription;
  s >> mSearchPattern;
}

Akonadi::Collection SearchDescriptionAttribute::baseCollection() const
{
  return mBaseCollection;
}

void SearchDescriptionAttribute::setBaseCollection(const Akonadi::Collection& collection)
{
  mBaseCollection = collection;
}

bool SearchDescriptionAttribute::recursive() const
{
  return mRecursive;
}

void SearchDescriptionAttribute::setRecursive(bool recursive)
{
  mRecursive = recursive;
}

QVariant SearchDescriptionAttribute::searchPattern() const
{
  return mSearchPattern;
}

void SearchDescriptionAttribute::setSearchPattern(const QVariant& searchPattern)
{
  mSearchPattern = searchPattern;
}

QString SearchDescriptionAttribute::description() const
{
  return mDescription;
}

void SearchDescriptionAttribute::setDescription(const QString& description)
{
  mDescription = description;
}












} /* namespace Akregator2 */
