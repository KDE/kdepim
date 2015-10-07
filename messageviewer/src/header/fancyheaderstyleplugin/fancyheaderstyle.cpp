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

#include "fancyheaderstyle.h"
#include "messageviewer_debug.h"
#include "header/headerstyle.h"
#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <KTextToHTML>
#include "settings/messageviewersettings.h"
#include "messageviewer/nodehelper.h"

#include <MessageCore/StringUtil>

#include <KLocalizedString>

#include <kcodecs.h>
#include <KColorScheme>

#include <QBuffer>
#include <QBitmap>
#include <QImage>
#include <QApplication>
#include <QRegExp>
#include <QFontMetrics>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;

using namespace MessageViewer;
//
// FancyHeaderStyle:
//   Like PlainHeaderStyle, but with slick frames and background colours.
//

FancyHeaderStyle::FancyHeaderStyle()
    : HeaderStyle()
{

}

FancyHeaderStyle::~FancyHeaderStyle()
{

}

const char *FancyHeaderStyle::name() const
{
    return "fancy";
}

QString FancyHeaderStyle::format(KMime::Message *message) const
{
    if (!message) {
        return QString();
    }
    const HeaderStrategy *strategy = headerStrategy();
    // ### from kmreaderwin begin
    // The direction of the header is determined according to the direction
    // of the application layout.

    const QString dir = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
    QString headerStr = QStringLiteral("<div class=\"fancy header\" dir=\"%1\">\n").arg(dir);

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    const QString subjectDir = MessageViewer::HeaderStyleUtil::subjectDirectionString(message);

    // Spam header display.
    // If the spamSpamStatus config value is true then we look for headers
    // from a few spam filters and try to create visually meaningful graphics
    // out of the spam scores.

    const QString spamHTML =  MessageViewer::HeaderStyleUtil::spamStatus(message);

    QString userHTML;
    MessageViewer::HeaderStyleUtil::xfaceSettings xface = MessageViewer::HeaderStyleUtil::xface(this, message);
    if (!xface.photoURL.isEmpty()) {
        //qCDebug(MESSAGEVIEWER_LOG) << "Got a photo:" << xface.photoURL;
        userHTML = QStringLiteral("<img src=\"%1\" width=\"%2\" height=\"%3\">")
                   .arg(xface.photoURL).arg(xface.photoWidth).arg(xface.photoHeight);
        userHTML = QLatin1String("<div class=\"senderpic\">") + userHTML + QLatin1String("</div>");
    }

    // the subject line and box below for details
    if (strategy->showHeader(QStringLiteral("subject"))) {
        KTextToHTML::Options flags = KTextToHTML::PreserveSpaces;
        if (MessageViewer::GlobalSettings::self()->showEmoticons()) {
            flags |= KTextToHTML::ReplaceSmileys;
        }

        headerStr += QStringLiteral("<div dir=\"%1\">%2</div>\n")
                     .arg(subjectDir)
                     .arg(MessageViewer::HeaderStyleUtil::subjectString(message, flags));
    }
    headerStr += QLatin1String("<table class=\"outer\"><tr><td width=\"100%\"><table>\n");
    //headerStr += "<table>\n";
    // from line
    // the mailto: URLs can contain %3 etc., therefore usage of multiple
    // QString::arg is not possible
    if (strategy->showHeader(QStringLiteral("from"))) {

        const QVector<KMime::Types::Mailbox> resentFrom = MessageViewer::HeaderStyleUtil::resentFromList(message);
        headerStr += QStringLiteral("<tr><th>%1</th>\n"
                                    "<td>")
                     .arg(i18n("From: "))
                     + StringUtil::emailAddrAsAnchor(message->from(), StringUtil::DisplayFullAddress)
                     + (message->headerByType("Resent-From") ? QLatin1String("&nbsp;")
                        + i18n("(resent from %1)",
                               StringUtil::emailAddrAsAnchor(
                                   resentFrom, StringUtil::DisplayFullAddress))
                        : QString())
                     + (!vCardName().isEmpty() ? QLatin1String("&nbsp;&nbsp;<a href=\"") + vCardName() + QLatin1String("\">")
                        + i18n("[vCard]") + QLatin1String("</a>")
                        : QString())
                     + (!message->organization(false)
                        ? QString()
                        : QLatin1String("&nbsp;&nbsp;(")
                        + MessageViewer::HeaderStyleUtil::strToHtml(message->organization()->asUnicodeString())
                        + QLatin1Char(')'))
                     + QLatin1String("</td></tr>\n");
    }
    // to line
    if (strategy->showHeader(QStringLiteral("to"))) {
        const QVector<KMime::Types::Mailbox> resentTo = MessageViewer::HeaderStyleUtil::resentToList(message);

        QString to;
        if (message->headerByType("Resent-To")) {
            to = StringUtil::emailAddrAsAnchor(resentTo, StringUtil::DisplayFullAddress) + QLatin1Char(' ') + i18n("(receiver was %1)", StringUtil::emailAddrAsAnchor(message->to(), StringUtil::DisplayFullAddress,
                    QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                    QStringLiteral("FullToAddressList")));
        } else {
            to = StringUtil::emailAddrAsAnchor(message->to(), StringUtil::DisplayFullAddress,
                                               QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                               QStringLiteral("FullToAddressList"));
        }

        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td>%2</td></tr>\n")
                         .arg(i18nc("To-field of the mail header.", "To: "))
                         .arg(to));
    }

    // cc line, if an
    if (strategy->showHeader(QStringLiteral("cc")) && message->cc(false))
        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td>%2</td></tr>\n")
                         .arg(i18n("CC: "))
                         .arg(StringUtil::emailAddrAsAnchor(message->cc(), StringUtil::DisplayFullAddress,
                                 QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                 QStringLiteral("FullCcAddressList"))));

    // Bcc line, if any
    if (strategy->showHeader(QStringLiteral("bcc")) && message->bcc(false))
        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td>%2</td></tr>\n")
                         .arg(i18n("BCC: "))
                         .arg(StringUtil::emailAddrAsAnchor(message->bcc(), StringUtil::DisplayFullAddress)));

    if (strategy->showHeader(QStringLiteral("date")))
        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td dir=\"%2\">%3</td></tr>\n")
                         .arg(i18n("Date: "))
                         .arg(MessageViewer::HeaderStyleUtil::directionOf(MessageViewer::HeaderStyleUtil::dateStr(message->date()->dateTime())))
                         .arg(MessageViewer::HeaderStyleUtil::strToHtml(MessageViewer::HeaderStyleUtil::dateString(message, isPrinting(), /* short = */ false))));
    if (MessageViewer::GlobalSettings::self()->showUserAgent()) {
        if (strategy->showHeader(QStringLiteral("user-agent"))) {
            if (auto hdr = message->userAgent(false)) {
                headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                                "<td>%2</td></tr>\n")
                                 .arg(i18n("User-Agent: "))
                                 .arg(MessageViewer::HeaderStyleUtil::strToHtml(hdr->asUnicodeString())));
            }
        }

        if (strategy->showHeader(QStringLiteral("x-mailer"))) {
            if (message->headerByType("X-Mailer")) {
                headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                                "<td>%2</td></tr>\n")
                                 .arg(i18n("X-Mailer: "))
                                 .arg(MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("X-Mailer")->asUnicodeString())));
            }
        }
    }

    if (strategy->showHeader(QStringLiteral("x-bugzilla-url")) && message->headerByType("X-Bugzilla-URL")) {
        const QString product   = message->headerByType("X-Bugzilla-Product")   ? message->headerByType("X-Bugzilla-Product")->asUnicodeString() : QString();
        const QString component = message->headerByType("X-Bugzilla-Component") ? message->headerByType("X-Bugzilla-Component")->asUnicodeString() : QString();
        const QString status    = message->headerByType("X-Bugzilla-Status")    ? message->headerByType("X-Bugzilla-Status")->asUnicodeString() : QString();
        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td>%2/%3, <strong>%4</strong></td></tr>\n")
                         .arg(i18n("Bugzilla: "))
                         .arg(MessageViewer::HeaderStyleUtil::strToHtml(product))
                         .arg(MessageViewer::HeaderStyleUtil::strToHtml(component))
                         .arg(MessageViewer::HeaderStyleUtil::strToHtml(status)));
    }

    if (strategy->showHeader(QStringLiteral("disposition-notification-to")) && message->headerByType("Disposition-Notification-To")) {
        const QString to = message->headerByType("Disposition-Notification-To") ? message->headerByType("Disposition-Notification-To")->asUnicodeString() : QString();
        headerStr.append(QStringLiteral("<tr><th>%1</th>\n"
                                        "<td>%2</tr>\n")
                         .arg(i18n("MDN To: "))
                         .arg(to));
    }

    headerStr.append(QLatin1String("<tr><td colspan=\"2\"><div id=\"attachmentInjectionPoint\"></div></td></tr>"));
    headerStr.append(
        QStringLiteral("</table></td><td align=\"center\">%1</td></tr></table>\n").arg(userHTML));

    if (!spamHTML.isEmpty())
        headerStr.append(QStringLiteral("<div class=\"spamheader\" dir=\"%1\"><b>%2</b>&nbsp;<span style=\"padding-left: 20px;\">%3</span></div>\n")
                         .arg(subjectDir, i18n("Spam Status:"), spamHTML));

    headerStr += QLatin1String("</div>\n\n");
    return headerStr;
}

bool FancyHeaderStyle::hasAttachmentQuickList() const
{
    return true;
}
