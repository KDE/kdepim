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

#ifndef MESSAGECORE_ATTACHMENTLOADJOB_H
#define MESSAGECORE_ATTACHMENTLOADJOB_H

#include "messagecore_export.h"

#include "attachmentpart.h"

#include <KJob>

namespace MessageCore
{

/**
 * @short A base class for jobs to load attachments from different sources.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 */
class MESSAGECORE_EXPORT AttachmentLoadJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new attachment load job.
     *
     * @param parent The parent object.
     */
    explicit AttachmentLoadJob(QObject *parent = 0);

    /**
     * Destroys the attachment load job.
     */
    virtual ~AttachmentLoadJob();

    /**
     * Starts the attachment load job.
     */
    virtual void start();

    /**
     * Returns the loaded attachment.
     */
    AttachmentPart::Ptr attachmentPart() const;

protected:
    /**
     * Subclasses use this method to set the loaded @p part.
     */
    void setAttachmentPart(const AttachmentPart::Ptr &part);

protected Q_SLOTS:
    virtual void doStart() = 0;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
