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

#include "finalmessage.h"
#include "finalmessage_p.h"

using namespace KMime;
using namespace MessageComposer;

FinalMessage::FinalMessage( Message *message )
  : d( new Private )
{
  d->message = Message::Ptr( message );
  d->hasCustomHeaders = false;
  d->transportId = -1;
}

FinalMessage::~FinalMessage()
{
  delete d;
}

Message::Ptr FinalMessage::message() const
{
  return d->message;
}

int FinalMessage::transportId() const
{
  return d->transportId;
}

bool FinalMessage::hasCustomHeaders() const
{
  return d->hasCustomHeaders;
}

QString FinalMessage::from() const
{
  return d->from;
}

QStringList FinalMessage::to() const
{
  return d->to;
}

QStringList FinalMessage::cc() const
{
  return d->cc;
}

QStringList FinalMessage::bcc() const
{
  return d->bcc;
}

