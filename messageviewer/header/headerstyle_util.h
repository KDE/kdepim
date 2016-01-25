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
#include <kpimutils/linklocator.h>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>
#include "antispam/spamheaderanalyzer.h"

#include "header/headerstyle.h"

using KPIMUtils::LinkLocator;

namespace MessageViewer {
namespace HeaderStyleUtil {
//
// Convenience functions:
//

enum HeaderStyleUtilDateFormat {
    ShortDate,        /**< Locale Short date format, e.g. 08-04-2007 */
    LongDate,         /**< Locale Long date format, e.g. Sunday 08 April 2007 */
    FancyShortDate,   /**< Same as ShortDate for dates a week or more ago. For more
                           recent dates, it is represented as Today, Yesterday, or
                           the weekday name. */
    FancyLongDate,    /**< Same as LongDate for dates a week or more ago. For more
                           recent dates, it is represented as Today, Yesterday, or
                           the weekday name. */
    CustomDate
};

QString directionOf( const QString &str );

QString strToHtml( const QString &str, int flags = LinkLocator::PreserveSpaces );

QString dateString( KMime::Message *message, bool printing, HeaderStyleUtilDateFormat dateFormat );

QString subjectString( KMime::Message *message, int flags = LinkLocator::PreserveSpaces );

QString subjectDirectionString( KMime::Message *message );

QString drawSpamMeter( SpamError spamError, double percent, double confidence,
                              const QString &filterHeader, const QString &confidenceHeader );

QString imgToDataUrl( const QImage &image );

QString spamStatus(KMime::Message *message);

QString dateStr(const KDateTime &dateTime);

QString dateShortStr(const KDateTime &dateTime);

QList<KMime::Types::Mailbox> resentFromList(KMime::Message *message);
QList<KMime::Types::Mailbox> resentToList(KMime::Message *message);

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

xfaceSettings xface(const HeaderStyle *style, KMime::Message *message);
void updateXFaceSettings(QImage photo, xfaceSettings &settings);

}
}


#endif // HEADERSTYLE_UTIL_H
