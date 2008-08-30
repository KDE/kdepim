/*
    This file is part of libqopensync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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
#include <opensync/opensync-format.h>

#include "objectformatsink.h"

using namespace QSync;

ObjectFormatSink::ObjectFormatSink()
  : mObjectFormatSink( 0 )
{
}

ObjectFormatSink::~ObjectFormatSink()
{
}

bool ObjectFormatSink::isValid() const
{
  return (mObjectFormatSink != 0);
}

QString ObjectFormatSink::objectFormat() const
{
  return QString::fromUtf8( osync_objformat_sink_get_objformat( mObjectFormatSink ) );
}

void ObjectFormatSink::setConfiguration( const QString &config )
{
  osync_objformat_sink_set_config( mObjectFormatSink, config.toUtf8().data() );
}

QString ObjectFormatSink::configuration() const
{
  return QString::fromUtf8( osync_objformat_sink_get_config( mObjectFormatSink ) );
}
