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

#include "grantleeheaderformatter.h"
#include "headerstyle_util.h"
#include "settings/messageviewersettings.h"
#include "config-messageviewer.h"

#include <MessageCore/StringUtil>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

#include <KLocalizedString>

#include <grantlee/templateloader.h>
#include <grantlee/engine.h>

using namespace MessageCore;

namespace MessageViewer
{

class Q_DECL_HIDDEN GrantleeHeaderFormatter::Private
{
public:
    Private()
    {
        engine = new Grantlee::Engine;
        templateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);
        engine->addTemplateLoader(templateLoader);
        engine->addDefaultLibrary(QStringLiteral("grantlee_messageheaderfilters"));
        engine->addTemplateLoader(templateLoader);
    }
    ~Private()
    {
        delete engine;
    }

    QSharedPointer<Grantlee::FileSystemTemplateLoader> templateLoader;
    Grantlee::Engine *engine;
};

GrantleeHeaderFormatter::GrantleeHeaderFormatter()
    : d(new GrantleeHeaderFormatter::Private)
{
}

GrantleeHeaderFormatter::~GrantleeHeaderFormatter()
{
    delete d;
}

QString GrantleeHeaderFormatter::toHtml(const QStringList &displayExtraHeaders, const QString &absolutPath, const QString &filename, const MessageViewer::HeaderStyle *style, KMime::Message *message, bool isPrinting) const
{
    d->templateLoader->setTemplateDirs(QStringList() << absolutPath);
    Grantlee::Template headerTemplate = d->engine->loadByName(filename);
    if (headerTemplate->error()) {
        return headerTemplate->errorString();
    }
    return format(absolutPath, headerTemplate, displayExtraHeaders, isPrinting, style, message);
}

QString GrantleeHeaderFormatter::toHtml(const GrantleeTheme::Theme &theme, bool isPrinting, const MessageViewer::HeaderStyle *style, KMime::Message *message) const
{
    QString errorMessage;
    if (!theme.isValid()) {
        errorMessage = i18n("Grantlee theme \"%1\" is not valid.", theme.name());
        return errorMessage;
    }
    d->templateLoader->setTemplateDirs(QStringList() << theme.absolutePath());
    Grantlee::Template headerTemplate = d->engine->loadByName(theme.themeFilename());
    if (headerTemplate->error()) {
        errorMessage = headerTemplate->errorString();
        return errorMessage;
    }
    return format(theme.absolutePath(), headerTemplate, theme.displayExtraVariables(), isPrinting, style, message);
}

QString GrantleeHeaderFormatter::format(const QString &absolutePath, const Grantlee::Template &headerTemplate, const QStringList &displayExtraHeaders, bool isPrinting, const MessageViewer::HeaderStyle *style, KMime::Message *message) const
{
    QVariantHash headerObject;

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.
    const QString absoluteThemePath = QStringLiteral("file://") + absolutePath + QLatin1Char('/');
    headerObject.insert(QStringLiteral("absoluteThemePath"), absoluteThemePath);
    headerObject.insert(QStringLiteral("applicationDir"), QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"));
    headerObject.insert(QStringLiteral("subjectDir"), MessageViewer::HeaderStyleUtil::subjectDirectionString(message));

    headerObject.insert(QStringLiteral("subjecti18n"), i18n("Subject:"));
    headerObject.insert(QStringLiteral("subject"), MessageViewer::HeaderStyleUtil::subjectString(message));

    headerObject.insert(QStringLiteral("toi18n"), i18n("To:"));
    headerObject.insert(QStringLiteral("to"), StringUtil::emailAddrAsAnchor(message->to(), StringUtil::DisplayFullAddress));
    headerObject.insert(QStringLiteral("toStr"), message->to()->asUnicodeString());
    const QString val = MessageCore::StringUtil::emailAddrAsAnchor(message->to(), MessageCore::StringUtil::DisplayFullAddress,
                        QString(), MessageCore::StringUtil::ShowLink,
                        MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList"));
    headerObject.insert(QStringLiteral("toExpandable"), val);
    headerObject.insert(QStringLiteral("toMailbox"), QVariant::fromValue(message->to()));

    if (message->replyTo(false)) {
        headerObject.insert(QStringLiteral("replyToi18n"), i18n("Reply to:"));
        headerObject.insert(QStringLiteral("replyTo"), StringUtil::emailAddrAsAnchor(message->replyTo(), StringUtil::DisplayFullAddress));
        headerObject.insert(QStringLiteral("replyToStr"), message->replyTo()->asUnicodeString());
    }

    if (message->cc(false)) {
        headerObject.insert(QStringLiteral("cci18n"), i18n("CC:"));
        headerObject.insert(QStringLiteral("cc"), StringUtil::emailAddrAsAnchor(message->cc(), StringUtil::DisplayFullAddress));
        headerObject.insert(QStringLiteral("ccStr"), message->cc()->asUnicodeString());
        headerObject.insert(QStringLiteral("ccMailbox"), QVariant::fromValue(message->cc()));
        const QString val = MessageCore::StringUtil::emailAddrAsAnchor(message->cc(), MessageCore::StringUtil::DisplayFullAddress,
                            QString(), MessageCore::StringUtil::ShowLink,
                            MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList"));
        headerObject.insert(QStringLiteral("ccExpandable"), val);
    }

    if (message->bcc(false)) {
        headerObject.insert(QStringLiteral("bcci18n"), i18n("BCC:"));
        headerObject.insert(QStringLiteral("bcc"), StringUtil::emailAddrAsAnchor(message->bcc(), StringUtil::DisplayFullAddress));
        headerObject.insert(QStringLiteral("bccStr"), message->bcc()->asUnicodeString());
        headerObject.insert(QStringLiteral("bccMailbox"), QVariant::fromValue(message->bcc()));
        const QString val = MessageCore::StringUtil::emailAddrAsAnchor(message->bcc(), MessageCore::StringUtil::DisplayFullAddress,
                            QString(), MessageCore::StringUtil::ShowLink,
                            MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList"));
        headerObject.insert(QStringLiteral("bccExpandable"), val);
    }
    headerObject.insert(QStringLiteral("fromi18n"), i18n("From:"));
    headerObject.insert(QStringLiteral("from"),  StringUtil::emailAddrAsAnchor(message->from(), StringUtil::DisplayFullAddress));
    headerObject.insert(QStringLiteral("fromStr"), message->from()->asUnicodeString());

    const QString spamHtml = MessageViewer::HeaderStyleUtil::spamStatus(message);
    if (!spamHtml.isEmpty()) {
        headerObject.insert(QStringLiteral("spamstatusi18n"), i18n("Spam Status:"));
        headerObject.insert(QStringLiteral("spamHTML"), spamHtml);
    }
    headerObject.insert(QStringLiteral("datei18n"), i18n("Date:"));

    headerObject.insert(QStringLiteral("dateshort"), MessageViewer::HeaderStyleUtil::strToHtml(MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, true)));
    headerObject.insert(QStringLiteral("datelong"), MessageViewer::HeaderStyleUtil::strToHtml(MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, false)));
    headerObject.insert(QStringLiteral("date"), MessageViewer::HeaderStyleUtil::dateStr(message->date()->dateTime()));

    if (MessageViewer::MessageViewerSettings::self()->showUserAgent()) {
        if (auto hdr = message->userAgent(false)) {
            headerObject.insert(QStringLiteral("useragent"), MessageViewer::HeaderStyleUtil::strToHtml(hdr->asUnicodeString()));
        }

        if (message->headerByType("X-Mailer")) {
            headerObject.insert(QStringLiteral("xmailer"), MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("X-Mailer")->asUnicodeString()));
        }
    }

    if (message->headerByType("Resent-From")) {
        headerObject.insert(QStringLiteral("resentfromi18n"), i18n("resent from"));
        const QVector<KMime::Types::Mailbox> resentFrom = MessageViewer::HeaderStyleUtil::resentFromList(message);
        headerObject.insert(QStringLiteral("resentfrom"), StringUtil::emailAddrAsAnchor(resentFrom, StringUtil::DisplayFullAddress));
    }

    if (message->headerByType("Resent-To")) {
        const QVector<KMime::Types::Mailbox> resentTo = MessageViewer::HeaderStyleUtil::resentToList(message);
        headerObject.insert(QStringLiteral("resenttoi18n"), i18np("receiver was", "receivers were", resentTo.count()));
        headerObject.insert(QStringLiteral("resentto"), StringUtil::emailAddrAsAnchor(resentTo, StringUtil::DisplayFullAddress));
    }

    if (auto organization = message->organization(false)) {
        headerObject.insert(QStringLiteral("organization"), MessageViewer::HeaderStyleUtil::strToHtml(organization->asUnicodeString()));
    }

    if (!style->vCardName().isEmpty()) {
        headerObject.insert(QStringLiteral("vcardname"), style->vCardName());
    }

    if (isPrinting) {
        //provide a bit more left padding when printing
        //kolab/issue3254 (printed mail cut at the left side)
        //Use it just for testing if we are in printing mode
        headerObject.insert(QStringLiteral("isprinting"), i18n("Printing mode"));
    }

    // colors depend on if it is encapsulated or not
    QColor fontColor(Qt::white);
    QString linkColor = QStringLiteral("white");
    const QColor activeColor = KColorScheme(QPalette::Active, KColorScheme::Selection).background().color();
    QColor activeColorDark = activeColor.dark(130);
    // reverse colors for encapsulated
    if (!style->isTopLevel()) {
        activeColorDark = activeColor.dark(50);
        fontColor = QColor(Qt::black);
        linkColor = QStringLiteral("black");
    }

    // 3D borders
    headerObject.insert(QStringLiteral("activecolordark"), activeColorDark.name());
    headerObject.insert(QStringLiteral("fontcolor"), fontColor.name());
    headerObject.insert(QStringLiteral("linkcolor"), linkColor);

    MessageViewer::HeaderStyleUtil::xfaceSettings xface = MessageViewer::HeaderStyleUtil::xface(style, message);
    if (!xface.photoURL.isEmpty()) {
        headerObject.insert(QStringLiteral("photowidth"), xface.photoWidth);
        headerObject.insert(QStringLiteral("photoheight"), xface.photoHeight);
        headerObject.insert(QStringLiteral("photourl"), xface.photoURL);
    }

    Q_FOREACH (QString header, displayExtraHeaders) {
        const QByteArray baHeader = header.toLocal8Bit();
        if (message->headerByType(baHeader)) {
            //Grantlee doesn't support '-' in variable name => remove it.
            header = header.remove(QLatin1Char('-'));
            headerObject.insert(header, message->headerByType(baHeader)->asUnicodeString());
        }
    }

    headerObject.insert(QStringLiteral("vcardi18n"), i18n("[vcard]"));

    QVariantHash mapping;
    mapping.insert(QStringLiteral("header"), headerObject);
    Grantlee::Context context(mapping);

    return headerTemplate->render(&context);
}

}
