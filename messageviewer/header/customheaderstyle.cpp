/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "customheaderstyle.h"
#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <KPIMUtils/kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;
#include "settings/globalsettings.h"
#include "viewer/nodehelper.h"

#include <KPIMUtils/kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <qdebug.h>
#include <KLocalizedString>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;
using KPIMUtils::LinkLocator;
using namespace MessageViewer;

namespace MessageViewer
{

//
// CustomHeaderStyle:
//   show every header field on a line by itself,
//   show subject larger
//
QString CustomHeaderStyle::format(KMime::Message *message) const
{
    if (!message) {
        return QString();
    }
    const HeaderStrategy *strategy = headerStrategy();
    if (!strategy) {
        strategy = HeaderStrategy::custom();
    }

    // The direction of the header is determined according to the direction
    // of the application layout.

    const QString dir = (QApplication::isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr"));

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    const QString subjectDir = MessageViewer::HeaderStyleUtil::subjectDirectionString(message);
    QString headerStr;

    const QStringList headersToDisplay = strategy->headersToDisplay();

    if ((strategy->defaultPolicy() == HeaderStrategy::Hide) ||
            (headersToDisplay.isEmpty() && strategy->defaultPolicy() == HeaderStrategy::Display)) {
        // crude way to emulate "all" headers - Note: no strings have
        // i18n(), so direction should always be ltr.
        headerStr = QLatin1String("<div class=\"header\" dir=\"ltr\">");
        const QStringList headersToHide = strategy->headersToHide();
        headerStr += formatAllMessageHeaders(message, headersToHide);
        return headerStr + QLatin1String("</div>");
    }

    headerStr = QString::fromLatin1("<div class=\"header\" dir=\"%1\">").arg(dir);

    Q_FOREACH (const QString &headerToDisplay, headersToDisplay) {
        if (headerToDisplay.toLower() == QLatin1String("subject")) {
            headerStr += QString::fromLatin1("<div dir=\"%1\"><b style=\"font-size:130%\">").arg(subjectDir) +
                         MessageViewer::HeaderStyleUtil::subjectString(message) + QLatin1String("</b></div>\n");
        } else if (headerToDisplay.toLower() == QLatin1String("date")) {
            headerStr.append(i18n("Date: ") + MessageViewer::HeaderStyleUtil::strToHtml(MessageViewer::HeaderStyleUtil::dateString(message, isPrinting(), /* short = */ false)) + QLatin1String("<br/>\n"));
        } else if (headerToDisplay.toLower() == QLatin1String("from")) {
            headerStr.append(i18n("From: ") +
                             StringUtil::emailAddrAsAnchor(message->from(), StringUtil::DisplayFullAddress, QString(), StringUtil::ShowLink));
            if (!vCardName().isEmpty())
                headerStr.append(QLatin1String("&nbsp;&nbsp;<a href=\"") + vCardName() +
                                 QLatin1String("\">") + i18n("[vCard]") + QLatin1String("</a>"));

            if (strategy->showHeader(QLatin1String("organization"))
                    && message->headerByType("Organization"))
                headerStr.append(QLatin1String("&nbsp;&nbsp;(") +
                                 MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("Organization")->asUnicodeString()) + QLatin1Char(')'));
            headerStr.append(QLatin1String("<br/>\n"));
        } else if (headerToDisplay.toLower() == QLatin1String("to")) {
            headerStr.append(i18nc("To-field of the mailheader.", "To: ") +
                             StringUtil::emailAddrAsAnchor(message->to(), StringUtil::DisplayFullAddress) + QLatin1String("<br/>\n"));
        } else if (headerToDisplay.toLower() == QLatin1String("cc")) {
            if (message->cc(false)) {
                headerStr.append(i18n("CC: ") +
                                 StringUtil::emailAddrAsAnchor(message->cc(), StringUtil::DisplayFullAddress) + QLatin1String("<br/>\n"));
            }
        } else if (headerToDisplay.toLower() == QLatin1String("bcc")) {
            if (message->bcc(false)) {
                headerStr.append(i18n("BCC: ") +
                                 StringUtil::emailAddrAsAnchor(message->bcc(), StringUtil::DisplayFullAddress) + QLatin1String("<br/>\n"));
            }
        } else if (headerToDisplay.toLower() == QLatin1String("reply-to")) {
            if (message->replyTo(false)) {
                headerStr.append(i18n("Reply to: ") +
                                 StringUtil::emailAddrAsAnchor(message->replyTo(), StringUtil::DisplayFullAddress) + QLatin1String("<br/>\n"));
            }
        } else {
            const QByteArray header = headerToDisplay.toLatin1();
            if (message->headerByType(header)) {
                headerStr.append(MessageViewer::HeaderStyleUtil::strToHtml(i18n("%1: ", headerToDisplay) + message->headerByType(header)->asUnicodeString()) + QLatin1String("<br/>\n"));
            }
        }
    }
    headerStr += QLatin1String("</div>\n");
    return headerStr;
}

QString CustomHeaderStyle::formatAllMessageHeaders(KMime::Message *message, const QStringList &headersToHide) const
{
    QByteArray head = message->head();
    KMime::Headers::Base *header = KMime::HeaderParsing::extractFirstHeader(head);
    QString result;
    while (header) {
        const QString headerType = QLatin1String(header->type());
        if (!headersToHide.contains(headerType) || !headersToHide.contains(headerType.toLower())) {
            result += MessageViewer::HeaderStyleUtil::strToHtml(headerType) + QLatin1String(": ") + header->asUnicodeString();
            result += QLatin1String("<br />\n");
        }
        delete header;
        header = KMime::HeaderParsing::extractFirstHeader(head);
    }
    return result;
}
}
