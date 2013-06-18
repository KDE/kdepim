/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "imagescalingutils.h"
#include "settings/messagecomposersettings.h"

#include <QFileInfo>

using namespace MessageComposer;

bool Utils::resizeImage(MessageCore::AttachmentPart::Ptr part)
{
    const QString filename = part->fileName();
    const QString pattern = MessageComposer::MessageComposerSettings::self()->filterSourcePattern();

    if (!pattern.isEmpty()) {
        //TODO use regexp ?
        const QStringList lstPattern = pattern.split(QLatin1Char(';'));
        Q_FOREACH (const QString& patternStr, lstPattern) {
            switch (MessageComposer::MessageComposerSettings::self()->filterSourceType()) {
            case MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter:
                break;
            case MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern:
                if (!filename.contains(patternStr)) {
                    return false;
                }
                break;
            case MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern:
                if (filename.contains(patternStr)) {
                    return false;
                }
                break;
            }
        }
    }

    if (MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormats()) {
        const QString formatsType = MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormatsType();
        if (!formatsType.isEmpty()) {
            const QStringList lstFormat = formatsType.split(QLatin1Char(';'));
            bool willResizeImage = false;
            Q_FOREACH (const QString& type, lstFormat) {
                if(QString::fromLatin1(part->mimeType()) == type) {
                    willResizeImage = true;
                    break;
                }
            }
            if(!willResizeImage)
                return false;
        }
    }

    if (MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled() ) {
        if (part->size() > MessageComposer::MessageComposerSettings::self()->skipImageLowerSize() *1024) {
            if (hasImage(part->mimeType()) ) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

void Utils::changeFileName(MessageCore::AttachmentPart::Ptr part)
{
    if (MessageComposer::MessageComposerSettings::self()->renameResizedImages()) {
        QString pattern = MessageComposer::MessageComposerSettings::self()->renameResizedImagesPattern();
        if (!pattern.isEmpty()) {
            const QString filename = part->fileName();
            pattern.replace(QLatin1String("%t"), QTime::currentTime().toString());
            pattern.replace(QLatin1String("%d"), QDate::currentDate().toString());
            pattern.replace(QLatin1String("%n"), filename); //Original name
            pattern.replace(QLatin1String("%e"), QFileInfo(filename).completeSuffix()); //Original extension

            const QString type = MessageComposer::MessageComposerSettings::self()->writeFormat();
            QString newExtension;
            if(type == QLatin1String("JPG")) {
                newExtension = QLatin1String("jpg");
            } else if(type == QLatin1String("PNG")) {
                newExtension = QLatin1String("png");
            }
            if (!newExtension.isEmpty())
                pattern.replace(QLatin1String("%x"), newExtension); //new Extension

            //Need to define pattern type.
            part->setFileName(pattern);
        }
    }
}

bool Utils::filterRecipients(const QStringList& recipients)
{
    if (recipients.isEmpty())
        return false;

    if (MessageComposer::MessageComposerSettings::self()->filterRecipientType() == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter) {
        return true;
    }

    const QString doNotResizeEmailsPattern = MessageComposer::MessageComposerSettings::self()->doNotResizeEmailsPattern();
    const QString resizeEmailsPattern = MessageComposer::MessageComposerSettings::self()->resizeEmailsPattern();
    if (doNotResizeEmailsPattern.isEmpty() && resizeEmailsPattern.isEmpty())
        return true;

    switch(MessageComposer::MessageComposerSettings::self()->filterRecipientType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter:
        return true;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern:
        if (resizeEmailsPattern.isEmpty())
            return false;
        Q_FOREACH( const QString& emails, recipients ) {
            if (!emails.contains(resizeEmailsPattern)) {
                return false;
            }
        }
        return true;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern:
        if (resizeEmailsPattern.isEmpty())
            return false;
        Q_FOREACH( const QString& emails, recipients ) {
            if (emails.contains(resizeEmailsPattern)) {
                return true;
            }
        }
        return false;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern:
        if (doNotResizeEmailsPattern.isEmpty())
            return false;
        Q_FOREACH( const QString& emails, recipients ) {
            if (!emails.contains(doNotResizeEmailsPattern)) {
                return false;
            }
        }
        return true;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern:
        if (doNotResizeEmailsPattern.isEmpty())
            return false;
        Q_FOREACH( const QString& emails, recipients ) {
            if (emails.contains(doNotResizeEmailsPattern)) {
                return true;
            }
        }
        return false;
    }

    return false;
}

bool Utils::hasImage(const QByteArray &mimetype)
{
    if ( mimetype == "image/gif" ||
         mimetype == "image/jpeg" ||
         mimetype == "image/png" ) {
        return true;
    }
    return false;
}

bool Utils::containsImage(const MessageCore::AttachmentPart::List &parts)
{
    foreach( MessageCore::AttachmentPart::Ptr part, parts ) {
        if (hasImage(part->mimeType())) {
            return true;
        }
    }
    return false;
}
