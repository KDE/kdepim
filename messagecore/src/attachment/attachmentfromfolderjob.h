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

#ifndef MESSAGECORE_ATTACHMENTFROMFOLDERJOB_H
#define MESSAGECORE_ATTACHMENTFROMFOLDERJOB_H

#include "MessageCore/AttachmentFromUrlBaseJob"

#include <KZip>

namespace MessageCore
{

class AttachmentFromFolderJob : public AttachmentFromUrlBaseJob
{

    Q_OBJECT

public:

    /**
    * Creates a new job.
    *
    * @param url The url of the folder that will be compressed and added as attachment.
    * @param parent The parent object.
    */

    explicit AttachmentFromFolderJob(const QUrl &url = QUrl(), QObject *parent = Q_NULLPTR);

    /**
    * Destroys the job.
    */

    ~AttachmentFromFolderJob();

    /**
     * Sets the @p compression method, either KZip::Deflate or KZip::NoCompression.
     */
    void setCompression(KZip::Compression compression);

    /**
     * Returns the compression method used
     */
    KZip::Compression compression() const;

protected Q_SLOTS:
    void doStart() Q_DECL_OVERRIDE;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
};

}
#endif // ATTACHMENTFROMFOLDER_H
