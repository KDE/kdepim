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

#include "attachmentloadjob.h"

#include <boost/shared_ptr.hpp>

#include <QtCore/QTimer>

using namespace MessageCore;

class MessageCore::AttachmentLoadJob::Private
{
  public:
    AttachmentPart::Ptr mPart;
};


AttachmentLoadJob::AttachmentLoadJob( QObject *parent )
  : KJob( parent ),
    d( new Private )
{
}

AttachmentLoadJob::~AttachmentLoadJob()
{
  delete d;
}

void AttachmentLoadJob::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

AttachmentPart::Ptr AttachmentLoadJob::attachmentPart() const
{
  return d->mPart;
}

void AttachmentLoadJob::setAttachmentPart( const AttachmentPart::Ptr &part )
{
  d->mPart = part;
}

#include "attachmentloadjob.moc"
