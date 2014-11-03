/* -*- mode: c++; c-basic-offset:4 -*-
    utils/cached.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEOPATRA_UTILS_CACHED_H__
#define __KLEOPATRA_UTILS_CACHED_H__

#include <boost/call_traits.hpp>

namespace Kleo
{

template <typename T>
class cached
{
    T m_value;
    bool m_dirty;
public:
    cached() : m_value(), m_dirty(true) {}
    /* implicit */ cached(typename boost::call_traits<T>::param_type value) : m_value(value), m_dirty(false) {}

    operator typename boost::call_traits<T>::param_type() const
    {
        return m_value;
    }

    cached &operator=(typename boost::call_traits<T>::param_type value)
    {
        m_value = value;
        m_dirty = false;
        return *this;
    }

    bool dirty() const
    {
        return m_dirty;
    }
    typename boost::call_traits<T>::param_type value() const
    {
        return m_value;
    }

    void set_dirty()
    {
        m_dirty = true;
    }
};

}

#endif /* __KLEOPATRA_UTILS_CACHED_H__ */
