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

#include <opensync/opensync.h>

#include "filter.h"

using namespace QSync;

Filter::Filter()
  : mFilter( 0 )
{
}

Filter::~Filter()
{
}

bool Filter::isValid() const
{
  return (mFilter != 0);
}

void Filter::setConfiguration( const QString &configuration )
{
  Q_ASSERT( mFilter );

  osync_filter_set_config( mFilter, (const char*)configuration.utf8() );
}

QString Filter::configuration() const
{
  Q_ASSERT( mFilter );

  return QString::fromUtf8( osync_filter_get_config( mFilter ) );
}

