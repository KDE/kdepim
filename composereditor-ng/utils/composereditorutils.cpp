/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "utils/composereditorutils_p.h"

#include <QFile>

QColor ComposerEditorNG::Utils::convertRgbToQColor(QString rgb)
{
    rgb.chop(1);
    rgb.remove(QLatin1String("rgb("));
    rgb = rgb.simplified();
    const QStringList colorLst = rgb.split(QLatin1Char(','));
    if (colorLst.count() == 3) {
        const QColor col(colorLst.at(0).toInt(),colorLst.at(1).toInt(),colorLst.at(2).toInt());
        return col;
    }
    return QColor();
}


QUrl ComposerEditorNG::Utils::guessUrlFromString(const QString &string)
{
    const QString urlStr = string.trimmed();
    const QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    const bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if (url.isValid())
            return url;
    }

    // Might be a file.
    if (QFile::exists(urlStr))
        return QUrl::fromLocalFile(urlStr);

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) {
        const int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            const QString prefix = urlStr.left(dotIndex).toLower();
            const QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }
    }

    // Fall back to QUrl's own tolerant parser.
    return QUrl(string, QUrl::TolerantMode);
}

