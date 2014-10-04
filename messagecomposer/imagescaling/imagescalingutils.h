/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef AUTORESIZEIMAGEUTIL_H
#define AUTORESIZEIMAGEUTIL_H
#include <messagecore/attachment/attachmentpart.h>

namespace MessageComposer
{
namespace Utils
{
bool containsImage(const MessageCore::AttachmentPart::List &parts);
bool resizeImage(MessageCore::AttachmentPart::Ptr part);
void changeFileName(MessageCore::AttachmentPart::Ptr part);
bool filterRecipients(const QStringList &recipients);
bool hasImage(const QByteArray &mimetype);
}

}

#endif // AUTORESIZEIMAGEUTIL_H
