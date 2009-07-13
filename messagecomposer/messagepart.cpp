/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "messagepart.h"

#include <KDebug>

using namespace KMime;
using namespace MessageComposer;

class MessagePart::Private
{
  public:
    Private()
      : autoCTE( true )
    {
    }

    bool autoCTE;
    Headers::contentEncoding cte;
};



MessagePart::MessagePart( QObject *parent )
  : QObject( parent )
  , d( new Private )
{
}

MessagePart::~MessagePart()
{
  delete d;
}

bool MessagePart::isAutoTransferEncoding() const
{
  return d->autoCTE;
}

KMime::Headers::contentEncoding MessagePart::overrideTransferEncoding() const
{
  if( d->autoCTE ) {
    kWarning() << "Called when CTE is auto.";
    return Headers::CEbinary;
  }
  return d->cte;
}

void MessagePart::setOverrideTransferEncoding( KMime::Headers::contentEncoding cte )
{
  d->autoCTE = false;
  d->cte = cte;
}

#include "messagepart.moc"
