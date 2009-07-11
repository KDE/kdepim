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

#include "attachmentpart.h"

#include <KUrl>

using namespace MessageComposer;

class MessageComposer::AttachmentPart::Private
{
  public:
    KUrl url;
    bool dataLoaded;
    QByteArray data;
};

AttachmentPart::AttachmentPart( QObject *parent )
  : MessagePart( parent )
  , d( new Private )
{
  d->dataLoaded = false;
}

AttachmentPart::~AttachmentPart()
{
  delete d;
}

KUrl AttachmentPart::url() const
{
  return d->url;
}

void AttachmentPart::setUrl( const KUrl &url )
{
  d->url = url;
  d->dataLoaded = false;
}

bool AttachmentPart::isDataLoaded() const
{
  return d->dataLoaded;
}

bool AttachmentPart::loadData()
{
  // TODO
  return false;
}

#include "attachmentpart.moc"
