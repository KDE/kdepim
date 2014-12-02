/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ATTACHMENTUPDATEJOB_H
#define ATTACHMENTUPDATEJOB_H

#include "messagecore_export.h"
#include <KJob>
#include "attachmentpart.h"

namespace MessageCore
{
class MESSAGECORE_EXPORT AttachmentUpdateJob : public KJob
{
    Q_OBJECT
public:
    AttachmentUpdateJob(const AttachmentPart::Ptr &part, QObject *parent = Q_NULLPTR);
    ~AttachmentUpdateJob();

    virtual void start();
    AttachmentPart::Ptr originalPart() const;
    AttachmentPart::Ptr updatedPart() const;
private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT(d, void doStart())
    Q_PRIVATE_SLOT(d, void loadJobResult(KJob *))
};
}

#endif // ATTACHMENTUPDATEJOB_H

