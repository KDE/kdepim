/*
    Copyright (C) 2011  Martin Bednár <serafean@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "attachmentfromurlbasejob.h"

using namespace MessageCore;

class AttachmentFromUrlBaseJob::Private {

  public:
    Private( AttachmentFromUrlBaseJob *qq );

     AttachmentFromUrlBaseJob *const q;
     qint64 mMaxSize;
     KUrl mUrl;
};

AttachmentFromUrlBaseJob::Private::Private( AttachmentFromUrlBaseJob* qq ):
    q( qq ),
    mMaxSize ( -1 )
{
}


AttachmentFromUrlBaseJob::AttachmentFromUrlBaseJob( const KUrl &url, QObject *parent ):
    AttachmentLoadJob( parent ),
    d( new Private( this ) )
{
  d->mUrl=url;
}

AttachmentFromUrlBaseJob::~AttachmentFromUrlBaseJob()
{
  delete d;
}

void AttachmentFromUrlBaseJob::setMaximumAllowedSize( qint64 size )
{
  d->mMaxSize = size;
}

qint64 AttachmentFromUrlBaseJob::maximumAllowedSize() const
{
  return d->mMaxSize;
}

void AttachmentFromUrlBaseJob::setUrl( const KUrl& url )
{
  d->mUrl = url;
}

KUrl AttachmentFromUrlBaseJob::url() const
{
  return d->mUrl;
}

