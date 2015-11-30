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

#include "scamdetection.h"
#include "scamdetectiondetailsdialog.h"
#include "scamcheckshorturl.h"
#include "settings/messageviewersettings.h"

#include <KLocalizedString>

#include <QRegularExpression>
#include <QWebElement>
#include <QWebFrame>

using namespace MessageViewer;
static QString addWarningColor(const QString &url)
{
    const QString error = QStringLiteral("<font color=#FF0000>%1</font>").arg(url);
    return error;
}
class MessageViewer::ScamDetectionPrivate
{
public:
    ScamDetectionPrivate(ScamDetection *q)
        : mCheckShortUrl(new ScamCheckShortUrl(q))
    {

    }
    QString mDetails;
    QPointer<MessageViewer::ScamDetectionDetailsDialog> mDetailsDialog;
    ScamCheckShortUrl *mCheckShortUrl;
};

ScamDetection::ScamDetection(QObject *parent)
    : QObject(parent),
      d(new MessageViewer::ScamDetectionPrivate(this))
{
}

ScamDetection::~ScamDetection()
{
    delete d;
}

ScamCheckShortUrl *ScamDetection::scamCheckShortUrl() const
{
    return d->mCheckShortUrl;
}

void ScamDetection::scanPage(QWebFrame *frame)
{
    if (MessageViewer::MessageViewerSettings::self()->scamDetectionEnabled()) {
        d->mDetails.clear();
        d->mDetails = QLatin1String("<b>") + i18n("Details:") + QLatin1String("</b><ul>");
        bool foundScam = false;
        const QWebElement rootElement = frame->documentElement();
        bool result = scanFrame(rootElement, d->mDetails);
        if (result) {
            foundScam = true;
        }
        foreach (QWebFrame *childFrame, frame->childFrames()) {
            result = scanFrame(childFrame->documentElement(), d->mDetails);
            if (result) {
                foundScam = true;
            }
        }
        if (foundScam) {
            Q_EMIT messageMayBeAScam();
        }
    }
}

bool ScamDetection::scanFrame(const QWebElement &rootElement, QString &details)
{
    bool foundScam = false;
    QRegularExpression ip4regExp(QStringLiteral("\\b[0-9]{1,3}\\.[0-9]{1,3}(?:\\.[0-9]{0,3})?(?:\\.[0-9]{0,3})?"));
    const QWebElementCollection allAnchor = rootElement.findAll(QStringLiteral("a"));
    Q_FOREACH (const QWebElement &anchorElement, allAnchor) {
        //1) detect if title has a url and title != href
        const QString href = anchorElement.attribute(QStringLiteral("href"));
        const QString title = anchorElement.attribute(QStringLiteral("title"));
        const QUrl url(href);
        if (!title.isEmpty()) {
            if (title.startsWith(QStringLiteral("http:"))
                    || title.startsWith(QStringLiteral("https:"))
                    || title.startsWith(QStringLiteral("www."))) {
                if (title.startsWith(QStringLiteral("www."))) {
                    const QString completUrl =  url.scheme() + QLatin1String("://") + title;
                    if (completUrl != href &&
                            href != (completUrl + QLatin1Char('/'))) {
                        foundScam = true;
                    }
                } else {
                    if (href != title) {
                        // http://www.kde.org == http://www.kde.org/
                        if (href != (title + QLatin1Char('/'))) {
                            foundScam = true;
                        }
                    }
                }
                if (foundScam) {
                    details += QLatin1String("<li>") + i18n("This email contains a link which reads as '%1' in the text, but actually points to '%2'. This is often the case in scam emails to mislead the recipient", addWarningColor(title), addWarningColor(href)) + QLatin1String("</li>");
                }
            }
        }
        if (!foundScam) {
            //2) detect if url href has ip and not server name.
            const QString hostname = url.host();
            if (hostname.contains(ip4regExp) && !hostname.contains(QStringLiteral("127.0.0.1"))) { //hostname
                details += QLatin1String("<li>") + i18n("This email contains a link which points to a numerical IP address (%1) instead of a typical textual website address. This is often the case in scam emails.", addWarningColor(hostname)) + QLatin1String("</li>");
                foundScam = true;
            } else if (hostname.contains(QLatin1Char('%'))) { //Hexa value for ip
                details += QLatin1String("<li>") + i18n("This email contains a link which points to a hexadecimal IP address (%1) instead of a typical textual website address. This is often the case in scam emails.", addWarningColor(hostname)) + QLatin1String("</li>");
                foundScam = true;
            } else if (url.toString().contains(QStringLiteral("url?q="))) { //4) redirect url.
                details += QLatin1String("<li>") + i18n("This email contains a link (%1) which has a redirection", addWarningColor(url.toString())) + QLatin1String("</li>");
                foundScam = true;
            } else if ((url.toString().count(QStringLiteral("http://")) > 1) ||
                       (url.toString().count(QStringLiteral("https://")) > 1)) { //5) more that 1 http in url.
                if (!url.toString().contains(QStringLiteral("kmail:showAuditLog"))) {
                    details += QLatin1String("<li>") + i18n("This email contains a link (%1) which contains multiple http://. This is often the case in scam emails.", addWarningColor(url.toString())) + QLatin1String("</li>");
                    foundScam = true;
                }
            }
        }
        //Check shortUrl
        if (!foundScam) {
            if (ScamCheckShortUrl::isShortUrl(url)) {
                details += QLatin1String("<li>") + i18n("This email contains a shorturl (%1). It can redirect to another server.", addWarningColor(url.toString())) + QLatin1String("</li>");
                foundScam = true;
            }
        }
    }
    //3) has form
    if (rootElement.findAll(QStringLiteral("form")).count() > 0) {
        details += QLatin1String("<li></b>") + i18n("Message contains form element. This is often the case in scam emails.") + QLatin1String("</b></li>");
        foundScam = true;
    }
    details += QLatin1String("</ul>");
    return foundScam;
}

void ScamDetection::showDetails()
{
    if (!d->mDetailsDialog) {
        d->mDetailsDialog = new MessageViewer::ScamDetectionDetailsDialog;
    }

    d->mDetailsDialog->setDetails(d->mDetails);
    d->mDetailsDialog->show();
}

