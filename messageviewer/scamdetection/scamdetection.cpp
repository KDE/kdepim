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
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QDebug>

using namespace MessageViewer;

ScamDetection::ScamDetection(QObject *parent)
    : QObject(parent)
{
}

ScamDetection::~ScamDetection()
{
}

void ScamDetection::scanPage(const QWebElement &rootElement)
{
    QWebElementCollection allAnchor = rootElement.findAll("a");
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
        //2) delete if url href has ip and not server name.
        //TODO
    }
}

#include "scamdetection.moc"
