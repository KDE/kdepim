/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef HEADERSTYLE_UTIL_H
#define HEADERSTYLE_UTIL_H

#include <QString>
#include <KTextToHTML>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>
#include "messageviewer/spamheaderanalyzer.h"

#include "messageviewer/headerstyle.h"
#include "messageviewer_export.h"
namespace MessageViewer
{
namespace HeaderStyleUtil
{
//
// Convenience functions:
//
MESSAGEVIEWER_EXPORT QString directionOf(const QString &str);

MESSAGEVIEWER_EXPORT QString strToHtml(const QString &str, KTextToHTML::Options flags = KTextToHTML::PreserveSpaces);

MESSAGEVIEWER_EXPORT QString dateString(KMime::Message *message, bool printing, bool shortDate);

MESSAGEVIEWER_EXPORT QString subjectString(KMime::Message *message, KTextToHTML::Options flags = KTextToHTML::PreserveSpaces);

MESSAGEVIEWER_EXPORT QString subjectDirectionString(KMime::Message *message);

QString drawSpamMeter(SpamError spamError, double percent, double confidence,
                      const QString &filterHeader, const QString &confidenceHeader);

QString imgToDataUrl(const QImage &image);

MESSAGEVIEWER_EXPORT QString spamStatus(KMime::Message *message);

MESSAGEVIEWER_EXPORT QString dateStr(const QDateTime &dateTime);

QString dateShortStr(const QDateTime &dateTime);

MESSAGEVIEWER_EXPORT QVector<KMime::Types::Mailbox> resentFromList(KMime::Message *message);
MESSAGEVIEWER_EXPORT QVector<KMime::Types::Mailbox> resentToList(KMime::Message *message);

struct xfaceSettings {
    xfaceSettings()
        : photoWidth(60),
          photoHeight(60)
    {
    }

    QString photoURL;
    int photoWidth;
    int photoHeight;
};

MESSAGEVIEWER_EXPORT xfaceSettings xface(const HeaderStyle *style, KMime::Message *message);
void updateXFaceSettings(QImage photo, xfaceSettings &settings);

}
}

#endif // HEADERSTYLE_UTIL_H
