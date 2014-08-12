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

#ifndef MESSAGECORE_ATTACHMENTCOMPRESSJOB_H
#define MESSAGECORE_ATTACHMENTCOMPRESSJOB_H

#include "messagecore_export.h"

#include "attachmentpart.h"

#include <KJob>

namespace MessageCore {

/**
 * @short A job to compress the attachment of an email.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 */
class MESSAGECORE_EXPORT AttachmentCompressJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new attachment compress job.
     *
     * @param part The part of the attachment to compress.
     * @param parent The parent object.
     */
    explicit AttachmentCompressJob( const AttachmentPart::Ptr &part, QObject *parent = 0 );

    /**
     * Destroys the attachment compress job.
     */
    virtual ~AttachmentCompressJob();

    /**
     * Starts the attachment compress job.
     */
    virtual void start();

    /**
     * Sets the original @p part of the compressed attachment.
     */
    void setOriginalPart( const AttachmentPart::Ptr &part );

    /**
     * Returns the original part of the compressed attachment.
     */
    const AttachmentPart::Ptr originalPart() const;

    /**
     * Returns the compressed part of the attachment.
     *
     * @note does not delete it unless it failed...
     */
    AttachmentPart::Ptr compressedPart() const;

    /**
     * Returns whether the compressed part is larger than the original part.
     */
    bool isCompressedPartLarger() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void doStart() )
    //@endcond
};

}

#endif
