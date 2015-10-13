/*
 * This file is part of the KDE project.
 * Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>
 * based on code from kwebkit part Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "networkaccessmanager.h"
#include "messageviewer_debug.h"
#include "settings/messageviewersettings.h"
#include "adblockmanager.h"

#include <KLocalizedString>
#include <KProtocolInfo>
#include <KRun>

#include <QTimer>
#include <QWidget>
#include <QNetworkReply>
#include <QWebFrame>
#include <QWebElementCollection>

#define HIDABLE_ELEMENTS   QLatin1String("audio,img,embed,object,iframe,frame,video")

/* Null network reply */
class NullNetworkReply : public QNetworkReply
{
public:
    NullNetworkReply(const QNetworkRequest &req, QObject *parent = Q_NULLPTR)
        : QNetworkReply(parent)
    {
        setRequest(req);
        setUrl(req.url());
        setHeader(QNetworkRequest::ContentLengthHeader, 0);
        setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/plain"));
        setError(QNetworkReply::ContentAccessDenied, i18n("Blocked by ad filter"));
        setAttribute(QNetworkRequest::User, QNetworkReply::ContentAccessDenied);
        QTimer::singleShot(0, this, &QNetworkReply::finished);
    }

    void abort() Q_DECL_OVERRIDE {}
    qint64 bytesAvailable() const Q_DECL_OVERRIDE
    {
        return 0;
    }

protected:
    qint64 readData(char *, qint64) Q_DECL_OVERRIDE {
        return -1;
    }
};

namespace MessageViewer
{

MyNetworkAccessManager::MyNetworkAccessManager(QObject *parent)
    : KIO::AccessManager(parent)
{
    auto lang = QLocale::system().language();
    QString c = QLocale(lang).name();

    if (c == QLatin1String("C")) {
        c = QStringLiteral("en-US");
    } else {
        c = c.replace(QLatin1Char('_'), QLatin1Char('-'));
    }

    c.append(QLatin1String(", en-US; q=0.8, en; q=0.6"));

    mAcceptLanguage = c.toLatin1();
}

QNetworkReply *MyNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    bool blocked = false;

    // Handle GET operations with AdBlock
    if (op == QNetworkAccessManager::GetOperation) {
        blocked = AdBlockManager::self()->blockRequest(req);
    }

    if (!blocked) {
        if (KProtocolInfo::isHelperProtocol(req.url())) {
            (void) new KRun(req.url(), qobject_cast<QWidget *>(req.originatingObject()));
            return new NullNetworkReply(req, this);
        }
        // set our "nice" accept-language header...
        QNetworkRequest request = req;
        request.setRawHeader("Accept-Language", mAcceptLanguage);

        return KIO::AccessManager::createRequest(op, req, outgoingData);
    }

    QWebFrame *frame = qobject_cast<QWebFrame *>(req.originatingObject());
    if (frame) {
        if (!mBlockedRequests.contains(frame)) {
            connect(frame, &QWebFrame::loadFinished, this, &MyNetworkAccessManager::slotApplyHidingBlockedElements);
        }
        mBlockedRequests.insert(frame, req.url());
    }

    return new NullNetworkReply(req, this);
}

static void hideBlockedElements(const QUrl &url, QWebElementCollection &collection)
{
    for (QWebElementCollection::iterator it = collection.begin(); it != collection.end(); ++it) {
        const QUrl baseUrl((*it).webFrame()->baseUrl());
        QString src = (*it).attribute(QStringLiteral("src"));
        if (src.isEmpty()) {
            src = (*it).evaluateJavaScript(QStringLiteral("this.src")).toString();
        }
        if (src.isEmpty()) {
            continue;
        }
        const QUrl resolvedUrl(baseUrl.resolved(src));
        if (url == resolvedUrl) {
            qCDebug(MESSAGEVIEWER_LOG) << "*** HIDING ELEMENT: " << (*it).tagName() << resolvedUrl;
            (*it).removeFromDocument();
        }
    }
}

void MyNetworkAccessManager::slotApplyHidingBlockedElements(bool ok)
{
    if (!ok) {
        return;
    }

    if (!MessageViewer::MessageViewerSettings::self()->adBlockEnabled()) {
        return;
    }

    if (!MessageViewer::MessageViewerSettings::self()->hideAdsEnabled()) {
        return;
    }

    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());
    if (!frame) {
        return;
    }

    QList<QUrl> urls = mBlockedRequests.values(frame);
    if (urls.isEmpty()) {
        return;
    }

    QWebElementCollection collection = frame->findAllElements(HIDABLE_ELEMENTS);
    if (frame->parentFrame()) {
        collection += frame->parentFrame()->findAllElements(HIDABLE_ELEMENTS);
    }

    Q_FOREACH (const QUrl &url, urls) {
        hideBlockedElements(url, collection);
    }
}

}

