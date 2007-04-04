/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef QSYNC_FILTER_H
#define QSYNC_FILTER_H

#include <qstring.h>

class OSyncFilter;

namespace QSync {

class Filter
{
  friend class Group;

  public:
    Filter();
    ~Filter();

    /**
      Returns whether the object is a valid filter.
     */
    bool isValid() const;

    /**
      Sets the configuration string of this filter. The format of
      string is filter specific.
     */
    void setConfiguration( const QString &configuration );

    /**
      Returns the configuration string of this filter.
     */
    QString configuration() const;

    bool operator==( const Filter &filter ) const { return mFilter == filter.mFilter; }

  private:
    OSyncFilter *mFilter;
};

}

#endif

