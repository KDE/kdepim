/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

/* This file is part of QWebdav
 *
 * Copyright (C) 2009-2010 Corentin Chary <corentin.chary@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QUrl>
#include "pimcommon_debug.h"

#include "webdav_url_info.h"

using namespace PimCommon;

QWebdavUrlInfo::QWebdavUrlInfo()
    : mIsDir(false),
      mIsFile(false)
{
    setSize(-1);
}

QWebdavUrlInfo::~QWebdavUrlInfo()
{
}

QWebdavUrlInfo::QWebdavUrlInfo(const QDomElement &dom)
{
    setSize(-1);
    QDomElement href = dom.namedItem(QStringLiteral("href")).toElement();

    mNode = dom.cloneNode();

    if (!href.isNull()) {
        const QString urlStr = QUrl::fromPercentEncoding(href.text().toUtf8());
        QDomNodeList propstats = dom.elementsByTagName(QStringLiteral("propstat"));
        davParsePropstats(urlStr, propstats);
    }
}

bool QWebdavUrlInfo::isValid() const
{
    return !mName.isEmpty();
}

int QWebdavUrlInfo::codeFromResponse(const QString &response)
{
    const int firstSpace = response.indexOf(QLatin1Char(' '));
    const int secondSpace = response.indexOf(QLatin1Char(' '), firstSpace + 1);
    return response.midRef(firstSpace + 1, secondSpace - firstSpace - 1).toInt();
}

QDateTime QWebdavUrlInfo::parseDateTime(const QString &input, const QString &type)
{
    QDateTime datetime;
    QLocale locale(QLocale::C);

    if (type == QLatin1String("dateTime.tz")) {
        datetime =  QDateTime::fromString(input, Qt::ISODate);
    } else if (type == QLatin1String("dateTime.rfc1123")) {
        datetime = locale.toDateTime(input);
    }

    if (!datetime.isNull()) {
        return datetime;
    }

    datetime = locale.toDateTime(input.left(19), QStringLiteral("yyyy-MM-dd'T'hh:mm:ss"));
    if (!datetime.isNull()) {
        return datetime;
    }
    datetime = locale.toDateTime(input.mid(5, 20), QStringLiteral("dd MMM yyyy hh:mm:ss"));
    if (!datetime.isNull()) {
        return datetime;
    }

    const QDate date = locale.toDate(input.mid(5, 11), QStringLiteral("dd MMM yyyy"));
    const QTime time = locale.toTime(input.mid(17, 8), QStringLiteral("hh:mm:ss"));
    return QDateTime(date, time);
}

void QWebdavUrlInfo::davParsePropstats(const QString &path, const QDomNodeList &propstats)
{
    QString mimeType;
    bool foundExecutable = false;
    bool isDirectory = false;

    setName(path);
    QMap<QString, QVariant> map;

    for (int i = 0; i < propstats.count(); ++i) {
        QDomElement propstat = propstats.item(i).toElement();
        QDomElement status = propstat.namedItem(QStringLiteral("status")).toElement();

        if (status.isNull()) {
            qCDebug(PIMCOMMON_LOG) << "Error, no status code in this propstat";
            return;
        }

        const int code = codeFromResponse(status.text());

        if (code == 404) {
            continue;
        }

        const QDomElement prop = propstat.namedItem(QStringLiteral("prop")).toElement();

        if (prop.isNull()) {
            qCDebug(PIMCOMMON_LOG) << "Error: no prop segment in this propstat.";
            return;
        }

        for (QDomNode n = prop.firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement property = n.toElement();

            if (property.isNull()) {
                continue;
            }

            //mProperties[property.namespaceURI()][property.tagName()] = property.text();

            if (property.namespaceURI() != QLatin1String("DAV:")) {
                // break out - we're only interested in properties from the DAV namespace
                continue;
            }
            map.insert(property.tagName(), property.text());

            if (property.tagName() == QLatin1String("creationdate")) {
                const QDateTime t = parseDateTime(property.text(), property.attribute(QStringLiteral("dt")));
                setCreatedAt(t);
                map.insert(QStringLiteral("creationdate"), t.toString());
            } else if (property.tagName() == QLatin1String("getcontentlength")) {
                setSize(property.text().toULong());
            } else if (property.tagName() == QLatin1String("displayname")) {
                setDisplayName(property.text());
            } else if (property.tagName() == QLatin1String("source")) {
                const QDomElement source = property.namedItem(QStringLiteral("link")).toElement().namedItem(QStringLiteral("dst")).toElement();
                if (!source.isNull()) {
                    setSource(source.text());
                }
            } else if (property.tagName() == QLatin1String("getcontentlanguage")) {
                setContentLanguage(property.text());
            } else if (property.tagName() == QLatin1String("getcontenttype")) {
                if (property.text() == QLatin1String("httpd/unix-directory")) {
                    isDirectory = true;
                } else {
                    mimeType = property.text();
                }
            } else if (property.tagName() == QLatin1String("executable")) {
                if (property.text() == QLatin1String("T")) {
                    foundExecutable = true;
                }
            } else if (property.tagName() == QLatin1String("getlastmodified")) {
                const QDateTime t = parseDateTime(property.text(), property.attribute(QStringLiteral("dt")));
                setLastModified(t);
                map.insert(QStringLiteral("lastmodified"), t.toString());
            } else if (property.tagName() == QLatin1String("getetag")) {
                setEntitytag(property.text());
            } else if (property.tagName() == QLatin1String("resourcetype")) {
                if (!property.namedItem(QStringLiteral("collection")).toElement().isNull()) {
                    isDirectory = true;
                }
            } else {
                qCDebug(PIMCOMMON_LOG) << "Found unknown webdav property: " << property.tagName() << property.text();
            }
        }
    }
    map.insert(QStringLiteral("isDir"), isDirectory);
    map.insert(QStringLiteral("path"), name());
    if (!map.isEmpty()) {
        mProperties = map;
    }

    setIsDir(isDirectory);
    setIsFile(!isDirectory);

    if (isDirectory && !name().endsWith(QLatin1String("/"))) {
        setName(name() + QLatin1String("/"));
    }
#if 0
    if (foundExecutable || isDirectory) {
        setPermissions(0700);
    } else {
        setPermissions(0600);
    }
#endif
    if (!isDirectory && !mimeType.isEmpty()) {
        setMimeType(mimeType);
    }
}

bool QWebdavUrlInfo::isFile() const
{
    return mIsFile;
}

void QWebdavUrlInfo::setIsFile(bool isFile)
{
    mIsFile = isFile;
}

qlonglong QWebdavUrlInfo::size() const
{
    return mSize;
}

void QWebdavUrlInfo::setSize(qlonglong size)
{
    mSize = size;
}

QDateTime QWebdavUrlInfo::lastModified() const
{
    return mLastModified;
}

void QWebdavUrlInfo::setLastModified(const QDateTime &lastModified)
{
    mLastModified = lastModified;
}

bool QWebdavUrlInfo::isDir() const
{
    return mIsDir;
}

void QWebdavUrlInfo::setIsDir(bool isDir)
{
    mIsDir = isDir;
}

QString QWebdavUrlInfo::name() const
{
    return mName;
}

void QWebdavUrlInfo::setName(const QString &name)
{
    mName = name;
}

void QWebdavUrlInfo::setCreatedAt(const QDateTime &date)
{
    mCreatedAt = date;
}

void QWebdavUrlInfo::setDisplayName(const QString &name)
{
    mDisplayName = name;
}

void QWebdavUrlInfo::setSource(const QString &source)
{
    mSource = source;
}

void QWebdavUrlInfo::setContentLanguage(const QString &lang)
{
    mContentLanguage = lang;
}

void QWebdavUrlInfo::setEntitytag(const QString &etag)
{
    mEntityTag = etag;
}

void QWebdavUrlInfo::setMimeType(const QString &mime)
{
    mMimeType = mime;
}

QDateTime QWebdavUrlInfo::createdAt() const
{
    return mCreatedAt;
}

QString QWebdavUrlInfo::displayName() const
{
    return mDisplayName;
}

QString QWebdavUrlInfo::source() const
{
    return mSource;
}

QString QWebdavUrlInfo::contentLanguage() const
{
    return mContentLanguage;
}

QString QWebdavUrlInfo::entityTag() const
{
    return mEntityTag;
}

QString QWebdavUrlInfo::mimeType() const
{
    return mMimeType;
}

QDomElement QWebdavUrlInfo::propElement() const
{
    return mNode.toElement();
}

const QVariantMap QWebdavUrlInfo::properties() const
{
    return mProperties;
}

QList<QWebdavUrlInfo> QWebdavUrlInfo::parseListInfo(const QString &lst)
{
    QList<QWebdavUrlInfo> lstUrlInfo;
    QDomDocument dom;
    dom.setContent(lst.toLatin1(), true);
    for (QDomNode n = dom.documentElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement thisResponse = n.toElement();

        if (thisResponse.isNull()) {
            continue;
        }

        QWebdavUrlInfo info(thisResponse);

        if (!info.isValid()) {
            continue;
        }
        lstUrlInfo.append(info);
    }
    return lstUrlInfo;
}
