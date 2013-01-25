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
#include "messagecomposersettings.h"

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

    if (MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled() &&
            (part->size() > MessageComposer::MessageComposerSettings::self()->skipImageLowerSize() *1024)) {
        if (part->mimeType() == "image/gif" ||
                part->mimeType() == "image/jpeg" ||
                part->mimeType() == "image/png" ) {
            return true;
        }
    }
    return false;
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
            //TODO
            pattern.replace(QLatin1String("%e"), filename); //Extension

            //TODO use pattern.
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
        return false;
    }

    const QString doNotResizeEmailsPattern = MessageComposer::MessageComposerSettings::self()->doNotResizeEmailsPattern();
    const QString resizeEmailsPattern = MessageComposer::MessageComposerSettings::self()->resizeEmailsPattern();
    if (doNotResizeEmailsPattern.isEmpty() && resizeEmailsPattern.isEmpty())
        return false;

    switch(MessageComposer::MessageComposerSettings::self()->filterRecipientType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter:
        return false;
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
