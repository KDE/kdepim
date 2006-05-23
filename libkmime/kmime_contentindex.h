/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

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

#ifndef KMIME_CONTENTINDEX_H
#define KMIME_CONTENTINDEX_H

#include <kdepim_export.h>

#include <QtCore/QList>
#include <QtCore/QString>

namespace KMime {

/**
  Index to uniquely identify message parts (Content object) in a part
  hierarchy. Basically a stack of integer indices.
  Based on RFC 3501 section 6.4.5 and thus compatible with IMAP.
*/
class KDE_EXPORT ContentIndex
{
  public:
    /**
      Create an empty (invalid) ContentIndex object.
    */
    ContentIndex();

    /**
      Create a ContentIndex object based on the given string representation.
      @param index A string representation of a message part index according
      to RFC 3501 section 6.4.5.
    */
    ContentIndex( const QString &index );

    /**
      Returns true if this index is non-empty (valid).
    */
    bool isValid() const;

    /**
      Removes and returns the top-most index. Usable to recursively
      descend into the message part hierarchy.
    */
    unsigned int pop();

    /**
      Adds an index to the ContentIndex. Usable when ascending the message
      part hierarchy.
      @param index Top-most part index.
    */
    void push( unsigned int index );

    /**
      Returns a string representation of this content index according
      to RFC 3501 section 6.4.5.
    */
    QString toString() const;

    /**
      Compares two content indices.
    */
    bool operator==( const ContentIndex& index ) const;

    /**
      Checks whether two content indices are not equal.
    */
    bool operator!=( const ContentIndex &index ) const;

  private:
    QList<unsigned int> mIndex;

};

}

#endif
