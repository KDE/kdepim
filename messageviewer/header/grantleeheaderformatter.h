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

#ifndef GRANTLEEHEADERFORMATTER_H
#define GRANTLEEHEADERFORMATTER_H

#include <QString>
#include <grantlee/templateloader.h>
namespace KMime {
class Message;
}

namespace MessageViewer {
class HeaderStyle;
class GrantleeTheme;
class GrantleeHeaderFormatter
{
public:
    explicit GrantleeHeaderFormatter();
    ~GrantleeHeaderFormatter();

    QString toHtml(const MessageViewer::GrantleeTheme &theme, bool isPrinting, const MessageViewer::HeaderStyle *style, KMime::Message *message) const;

    QString toHtml(const QStringList &displayExtraHeaders, const QString &absolutPath, const QString &filename, const MessageViewer::HeaderStyle *style, KMime::Message *message) const;
private:
    QString format(Grantlee::Template headerTemplate, const QStringList &displayExtraHeaders, bool isPrinting, const MessageViewer::HeaderStyle *style, KMime::Message *message) const;
    class Private;
    Private *const d;
};
}

#endif // GRANTLEEHEADERFORMATTER_H
