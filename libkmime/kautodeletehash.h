/*
    This file is part of libkdepim.

    Copyright (c) 2005 Ingo Kloecker <kloecker@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KPIM_KAUTODELETEHASH__
#define __KPIM_KAUTODELETEHASH__

#include <QHash>

/** \brief KPIM holds all kinds of functions specific to KDE PIM. */
namespace KPIM {

/**
 * The KAutoDeleteHash class is a convenience QHash subclass that provides
 * automatic deletion of the values in the destructor. Apart from this
 * KAutoDeleteHash behaves exactly like QHash<Key, T *>.
 *
 * Since the automatic deletion is restricted to the destruction of the hash
 * you have take care of the deletion of values you remove or replace yourself.
 * To replace a value in the hash by another value use
 * @code
 *   delete hash.take( key );
 *   hash.insert( key, value );
 * @endcode
 * and to remove a value from the hash use
 * @code
 *   delete hash.take( key );
 * @endcode
 *
 * @author Ingo Kl&ouml;cker <kloecker@kde.org>
 */
template <class Key, class T>
class KAutoDeleteHash : public QHash<Key, T *>
{
public:
  /**
   * Constructs an empty hash.
   */
  KAutoDeleteHash() {}
  /**
   * Constructs a copy of @p other (which can be a QHash or a KAutoDeleteHash).
   */
  KAutoDeleteHash( const QHash<Key, T *> &other ) : QHash<Key, T *>( other ) {}

  /**
   * Destroys the hash and deletes all values. References to the values in the
   * hash and all iterators of this hash become invalid.
   */
  ~KAutoDeleteHash() { while ( ! isEmpty() ) {
                         T *value = *begin();
                         erase( begin() );
                         delete value;
                       }
                     }
};

} // namespace KPIM

#endif /* __KPIM_KAUTODELETEHASH__ */
