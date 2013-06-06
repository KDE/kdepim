/*
 *  autoqpointer.h  -  QPointer which on destruction deletes object
 *  This is a (mostly) verbatim, private copy of kdepim/kalarm/lib/autoqpointer.h
 *
 *  Copyright © 2009 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MESSAGEVIEWER_AUTOQPOINTER_H
#define MESSAGEVIEWER_AUTOQPOINTER_H

#include <QPointer>
#include <QObject>

#include <kdemacros.h>

namespace MessageViewer {

/**
 *  A QPointer which when destructed, deletes the object it points to.
 *
 *  @author David Jarvie <djarvie@kde.org>, Marc Mutz <mutz@kde.org>
 */
template <class T>
class AutoQPointer
{
  Q_DISABLE_COPY( AutoQPointer )
  struct SafeBool { void func() {} };
  typedef void (SafeBool::*save_bool)();
  QPointer<QObject> o;
public:
    AutoQPointer() : o() {}
    explicit AutoQPointer( T * p ) : o(p) {}
    ~AutoQPointer()  { delete o; }
    T * data() const { return static_cast<T*>(o.data()); }
    T * get()  const { return data(); }
    bool isNull() const { return o.isNull(); }
    T * operator->() const { return data(); }
    T & operator*() const { return *data(); }
#if 0 // enable when all users of operator T*() have been converted to use .get()
    operator save_bool() const { return isNull() ? 0 : &SafeBool::func ; }
#else
    // unsafe - use explicit .get()
    KDE_DEPRECATED operator T*() const { return get(); }
    // unsafe - only provided to prevent the above warning in bool contexts
    operator bool() const { return get(); }
#endif
    
};

}

#endif // MESSAGEVIEWER_AUTOQPOINTER_H
