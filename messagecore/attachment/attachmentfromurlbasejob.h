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

#ifndef MESSAGECORE_ATTACHMENTFROMURLBASE_H
#define MESSAGECORE_ATTACHMENTFROMURLBASE_H

#include "messagecore_export.h"

#include "attachmentloadjob.h"

#include <KUrl>

namespace MessageCore
{

class MESSAGECORE_EXPORT AttachmentFromUrlBaseJob : public AttachmentLoadJob
{
    Q_OBJECT

public:
    explicit AttachmentFromUrlBaseJob(const KUrl &url = KUrl(), QObject *parent = 0);
    virtual ~AttachmentFromUrlBaseJob();

    /**
     * Returns the url that will be loaded as attachment.
     */
    KUrl url() const;

    /**
     * Returns the maximum size the attachment is allowed to have.
     */
    qint64 maximumAllowedSize() const;

    /**
    * Sets the @p url of the folder that will be loaded as attachment.
    */
    void setUrl(const KUrl &url);

    /**
     * Sets the maximum @p size the attachment is allowed to have.
     */
    void setMaximumAllowedSize(qint64 size);

protected Q_SLOTS:
    virtual void doStart() = 0;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
};
}
#endif // ATTACHMENTFROMURLBASE_H
