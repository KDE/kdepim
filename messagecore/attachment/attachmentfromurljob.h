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

#ifndef MESSAGECORE_ATTACHMENTFROMURLJOB_H
#define MESSAGECORE_ATTACHMENTFROMURLJOB_H

#include "messagecore_export.h"

#include "attachmentfromurlbasejob.h"

namespace MessageCore
{

/**
 * @short A job to load an attachment from an url.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 */
class MESSAGECORE_EXPORT AttachmentFromUrlJob : public AttachmentFromUrlBaseJob
{
    Q_OBJECT

public:
    /**
     * Creates a new job.
     *
     * @param url The url that will be loaded as attachment.
     * @param parent The parent object.
     */
    explicit AttachmentFromUrlJob(const QUrl &url = QUrl(), QObject *parent = Q_NULLPTR);

    /**
     * Destroys the job.
     */
    ~AttachmentFromUrlJob();

protected Q_SLOTS:
    void doStart();

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void transferJobData(KIO::Job *, QByteArray))
    Q_PRIVATE_SLOT(d, void transferJobResult(KJob *))
    //@endcond
};

}

#endif
