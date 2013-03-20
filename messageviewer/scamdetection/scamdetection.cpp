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
    : QObject(parent), mPage(new QWebPage())
{
    mPage->settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
    mPage->settings()->setAttribute( QWebSettings::JavaEnabled, false );
    mPage->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
    mPage->settings()->setAttribute( QWebSettings::AutoLoadImages, false );
}

ScamDetection::~ScamDetection()
{
    delete mPage;
}

void ScamDetection::scanPage(const QString &html)
{
    QWebFrame *frame = mPage->mainFrame();
    frame->setHtml( html, QUrl( "file:///" ) );
    QWebElement document = frame->documentElement();
    QWebElementCollection allAnchor = document.findAll("a");
    Q_FOREACH (const QWebElement &anchorElement, allAnchor) {
        qDebug()<<" href"<<anchorElement.attribute(QLatin1String("href"));
    }
}

#include "scamdetection.moc"
