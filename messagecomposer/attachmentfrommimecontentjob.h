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

#ifndef MESSAGECOMPOSER_ATTACHMENTFROMMIMECONTENT_H
#define MESSAGECOMPOSER_ATTACHMENTFROMMIMECONTENT_H

#include "jobbase.h"
#include "messagecomposer_export.h"

namespace KMime {
  class Content;
}

namespace MessageComposer {

class AttachmentFromMimeContentJobPrivate;
class AttachmentPart;

/**
*/
class MESSAGECOMPOSER_EXPORT AttachmentFromMimeContentJob : public JobBase
{
  Q_OBJECT

  public:
    explicit AttachmentFromMimeContentJob( const KMime::Content *content, QObject *parent = 0 );
    virtual ~AttachmentFromMimeContentJob();

    virtual void start();

    const KMime::Content *mimeContent() const;
    void setMimeContent( const KMime::Content *content );

    /// does not delete it unless it failed...
    AttachmentPart *attachmentPart() const;

  private:
    Q_DECLARE_PRIVATE( AttachmentFromMimeContentJob )

    Q_PRIVATE_SLOT( d_func(), void doStart() )
};

} // namespace MessageComposer

#endif
