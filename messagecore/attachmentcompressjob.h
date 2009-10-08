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

#ifndef KDEPIM_ATTACHMENTCOMPRESSJOB_H
#define KDEPIM_ATTACHMENTCOMPRESSJOB_H

#include "attachmentpart.h"
#include "messagecore_export.h"

#include <KDE/KJob>

namespace KPIM {

/**
*/
class MESSAGECORE_EXPORT AttachmentCompressJob : public KJob
{
  Q_OBJECT

  public:
    explicit AttachmentCompressJob( const AttachmentPart::Ptr &part, QObject *parent = 0 );
    virtual ~AttachmentCompressJob();

    virtual void start();

    const AttachmentPart::Ptr originalPart() const;
    void setOriginalPart( const AttachmentPart::Ptr part );
    /// does not delete it unless it failed...
    AttachmentPart::Ptr compressedPart() const;
    bool isCompressedPartLarger() const;

  private:
    class Private;
    friend class Private;
    Private *const d;
    Q_PRIVATE_SLOT( d, void doStart() )
};

} // namespace KPIM

#endif
