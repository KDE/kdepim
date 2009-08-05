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

#ifndef MESSAGECOMPOSER_ATTACHMENTCOMPRESSJOB_H
#define MESSAGECOMPOSER_ATTACHMENTCOMPRESSJOB_H

#include "jobbase.h"
#include "messagecomposer_export.h"

namespace MessageComposer {

class AttachmentCompressJobPrivate;
class AttachmentPart;

/**
*/
class MESSAGECOMPOSER_EXPORT AttachmentCompressJob : public JobBase
{
  Q_OBJECT

  public:
    explicit AttachmentCompressJob( const AttachmentPart *part, QObject *parent = 0 );
    virtual ~AttachmentCompressJob();

    virtual void start();

    const AttachmentPart *originalPart() const;
    void setOriginalPart( const AttachmentPart *part );
    /// does not delete it unless it failed...
    AttachmentPart *compressedPart() const;

    // default true
    bool warnCompressedSizeLarger() const;
    void setWarnCompressedSizeLarger( bool warn );

  private:
    Q_DECLARE_PRIVATE( AttachmentCompressJob )

    Q_PRIVATE_SLOT( d_func(), void doStart() )
};

} // namespace MessageComposer

#endif
