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

#include "scamdetection.h"
#include "globalsettings.h"
#include <QWebElement>
#include <QDebug>

using namespace MessageViewer;
static QString IPv4_PATTERN = QLatin1String("[0-9]{1,3}\\.[0-9]{1,3}(?:\\.[0-9]{0,3})?(?:\\.[0-9]{0,3})?");

ScamDetection::ScamDetection(QObject *parent)
    : QObject(parent)
{
}

ScamDetection::~ScamDetection()
{
}

void ScamDetection::scanPage(const QWebElement &rootElement)
{
    if (GlobalSettings::self()->scamDetectionEnabled()) {
        QRegExp ip4regExp;
        ip4regExp.setPattern(IPv4_PATTERN);
        QWebElementCollection allAnchor = rootElement.findAll(QLatin1String("a"));
        Q_FOREACH (const QWebElement &anchorElement, allAnchor) {
            //1) detect if title has a url and title != href
            const QString href = anchorElement.attribute(QLatin1String("href"));
            const QString title = anchorElement.attribute(QLatin1String("title"));
            if (!title.isEmpty()) {
                if (title.startsWith(QLatin1String("http:")) || title.startsWith(QLatin1String("https:"))) {
                    if (href != title) {
                        Q_EMIT messageMayBeAScam();
                        break;
                    }
                }
            }
            //2) detect if url href has ip and not server name.
            const QUrl url(href);
            if (url.host().contains(ip4regExp)) {
                Q_EMIT messageMayBeAScam();
                break;
            } else if (url.path().contains(QLatin1String("url?q="))) { //4) redirect url.
                Q_EMIT messageMayBeAScam();
                break;
            } else if (url.path().count(QLatin1String("http://"))>1) { //5) more that 1 http in url.
                Q_EMIT messageMayBeAScam();
                break;
            }
        }
        //3) has form
        if (rootElement.findAll(QLatin1String("form")).count() > 0) {
            Q_EMIT messageMayBeAScam();
            return;
        }
    }
}



#include "scamdetection.moc"
