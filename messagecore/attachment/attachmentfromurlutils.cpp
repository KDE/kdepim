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

#include "attachmentfromfolderjob.h"
#include "attachmentfromurljob.h"
#include "attachmentfromurlutils.h"
#include "messagecore/settings/globalsettings.h"
#include <KMimeType>
#include <QDebug>

namespace MessageCore {
MessageCore::AttachmentFromUrlBaseJob *AttachmentFromUrlUtils::createAttachmentJob(const KUrl &url, QObject *parent)
{
    MessageCore::AttachmentFromUrlBaseJob *ajob = 0;
    if( KMimeType::findByUrl( url )->name() == QLatin1String( "inode/directory" ) ) {
        qDebug() << "Creating attachment from folder";
        ajob = new MessageCore::AttachmentFromFolderJob ( url, parent );
    } else {
        ajob = new MessageCore::AttachmentFromUrlJob( url, parent );
        qDebug() << "Creating attachment from file";
    }
    if( MessageCore::GlobalSettings::maximumAttachmentSize() > 0 ) {
        ajob->setMaximumAllowedSize( MessageCore::GlobalSettings::maximumAttachmentSize() );
    }
    return ajob;
}
}
