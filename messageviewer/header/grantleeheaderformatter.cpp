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
#include "grantleeheaderstyle.h"
#include "headerstyle_util.h"
#include "settings/globalsettings.h"
#include "config-messageviewer.h"

#include <kpimutils/email.h>
#include <messagecore/utils/stringutil.h>
#include <utils/iconnamecache.h>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

#include <KLocalizedString>
#include <KStandardDirs>
#include <kiconloader.h>

#include <grantlee/templateloader.h>
#include <grantlee/engine.h>

using namespace MessageCore;

namespace MessageViewer {

class GrantleeHeaderFormatter::Private
{
public:
    Private()
    {
        engine = new Grantlee::Engine;
        const QStringList pluginDirs = KGlobal::dirs()->resourceDirs("qtplugins");
        foreach(const QString & pluginDir, pluginDirs) {
            engine->addPluginPath(pluginDir);
        }
        engine->addDefaultLibrary( QLatin1String("grantlee_messageheaderfilters") );
        templateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
        engine->addTemplateLoader( templateLoader );
    }
    ~Private()
    {
        delete engine;
    }

    Grantlee::FileSystemTemplateLoader::Ptr templateLoader;
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
    d->templateLoader->setTemplateDirs( QStringList() << absolutPath );
    Grantlee::Template headerTemplate = d->engine->loadByName( filename );
    if ( headerTemplate->error() ) {
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
    d->templateLoader->setTemplateDirs( QStringList() << theme.absolutePath() );
    Grantlee::Template headerTemplate = d->engine->loadByName( theme.themeFilename() );
    if ( headerTemplate->error() ) {
        errorMessage = headerTemplate->errorString();
        return errorMessage;
    }
    return format(theme.absolutePath(), headerTemplate, theme.displayExtraVariables(), isPrinting, style, message);
}

QString GrantleeHeaderFormatter::format(const QString &absolutePath, Grantlee::Template headerTemplate, const QStringList &displayExtraHeaders, bool isPrinting, const MessageViewer::HeaderStyle *style, KMime::Message *message) const
{
    QVariantHash headerObject;

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.
    const QString absoluteThemePath = QLatin1String("file://") + absolutePath + QLatin1Char('/');
    headerObject.insert(QLatin1String("absoluteThemePath"), absoluteThemePath);
    headerObject.insert(QLatin1String("applicationDir"), QApplication::isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr"));
    headerObject.insert(QLatin1String("subjectDir"), MessageViewer::HeaderStyleUtil::subjectDirectionString( message ));

    headerObject.insert(QLatin1String("subjecti18n"), i18n("Subject:") );
    headerObject.insert(QLatin1String("subject"), MessageViewer::HeaderStyleUtil::subjectString( message ) );

    if ( message->to( false )) {
        headerObject.insert(QLatin1String("toi18n"), i18n("To:") );
        headerObject.insert(QLatin1String("to"), StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("toNameOnly"), StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayNameOnly ));
        headerObject.insert(QLatin1String("toStr"), message->to()->asUnicodeString());
        const QString val = MessageCore::StringUtil::emailAddrAsAnchor( message->to(), MessageCore::StringUtil::DisplayFullAddress,
                                                                        QString(), MessageCore::StringUtil::ShowLink,
                                                                        MessageCore::StringUtil::ExpandableAddresses, QLatin1String("FullToAddressList"),
                                                                        GlobalSettings::self()->numberOfAddressesToShow() );
        headerObject.insert(QLatin1String("toExpandable"), val);
        headerObject.insert(QLatin1String("toMailbox"), QVariant::fromValue(message->to()));
    }

    if ( message->replyTo( false )) {
        headerObject.insert(QLatin1String("replyToi18n"), i18n("Reply to:") );
        headerObject.insert(QLatin1String("replyTo"), StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("replyToNameOnly"), StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayNameOnly ));
        headerObject.insert(QLatin1String("replyToStr"), message->replyTo()->asUnicodeString());
    }

    if ( message->cc( false ) ) {
        headerObject.insert(QLatin1String("cci18n"), i18n("CC:") );
        headerObject.insert(QLatin1String("cc"), StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("ccStr"), message->cc()->asUnicodeString());
        headerObject.insert(QLatin1String( "ccNameOnly" ) ,  StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayNameOnly ) );
        headerObject.insert(QLatin1String("ccMailbox"), QVariant::fromValue(message->cc()));
        const QString val = MessageCore::StringUtil::emailAddrAsAnchor( message->cc(), MessageCore::StringUtil::DisplayFullAddress,
                                                                         QString(), MessageCore::StringUtil::ShowLink,
                                                                         MessageCore::StringUtil::ExpandableAddresses, QLatin1String("FullToAddressList"),
                                                                         GlobalSettings::self()->numberOfAddressesToShow() );
        headerObject.insert(QLatin1String("ccExpandable"), val);
    }

    if ( message->bcc( false ) ) {
        headerObject.insert(QLatin1String("bcci18n"), i18n("BCC:"));
        headerObject.insert(QLatin1String("bcc"), StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String( "bccNameOnly" ) ,  StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayNameOnly ) );
        headerObject.insert(QLatin1String("bccStr"), message->bcc()->asUnicodeString());
        headerObject.insert(QLatin1String("bccMailbox"), QVariant::fromValue(message->bcc()));
        const QString val = MessageCore::StringUtil::emailAddrAsAnchor( message->bcc(), MessageCore::StringUtil::DisplayFullAddress,
                                                                         QString(), MessageCore::StringUtil::ShowLink,
                                                                         MessageCore::StringUtil::ExpandableAddresses, QLatin1String("FullToAddressList"),
                                                                         GlobalSettings::self()->numberOfAddressesToShow() );
        headerObject.insert(QLatin1String("bccExpandable"), val);
    }
    headerObject.insert(QLatin1String("fromi18n"), i18n("From:"));
    headerObject.insert(QLatin1String( "from" ) ,  StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress ) );
    headerObject.insert(QLatin1String( "fromNameOnly" ) ,  StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly ) );
    headerObject.insert(QLatin1String( "fromStr" ) , message->from()->asUnicodeString() );


    const QString spamHtml = MessageViewer::HeaderStyleUtil::spamStatus(message);
    if ( !spamHtml.isEmpty() ) {
        headerObject.insert( QLatin1String("spamstatusi18n"), i18n("Spam Status:"));
        headerObject.insert( QLatin1String( "spamHTML" ), spamHtml );
    }
    headerObject.insert(QLatin1String("datei18n"), i18n("Date:"));

    headerObject.insert( QLatin1String( "dateshort" ) ,
                         MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, MessageViewer::HeaderStyleUtil::ShortDate ) ) );

    headerObject.insert( QLatin1String( "datefancylong" ) ,
                         MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, MessageViewer::HeaderStyleUtil::FancyLongDate ) ) );
    headerObject.insert( QLatin1String( "datefancyshort" ) ,
                         MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, MessageViewer::HeaderStyleUtil::FancyShortDate ) ) );
    headerObject.insert( QLatin1String( "datelocalelong" ) ,
                         MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, MessageViewer::HeaderStyleUtil::LongDate ) ) );

    headerObject.insert( QLatin1String( "datelong" ) ,
                         MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting, MessageViewer::HeaderStyleUtil::CustomDate ) ) );
    headerObject.insert( QLatin1String( "date" ), MessageViewer::HeaderStyleUtil::dateStr( message->date()->dateTime() ) );

    if ( GlobalSettings::self()->showUserAgent() ) {
        if ( message->headerByType("User-Agent") ) {
            headerObject.insert( QLatin1String( "useragent" ), MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("User-Agent")->asUnicodeString() ) );
        }

        if ( message->headerByType("X-Mailer") ) {
            headerObject.insert( QLatin1String( "xmailer" ), MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("X-Mailer")->asUnicodeString() ) );
        }
    }

    if ( message->headerByType( "Resent-From" ) ) {
        headerObject.insert( QLatin1String( "resentfromi18n"), i18n("resent from"));
        const QList<KMime::Types::Mailbox> resentFrom = MessageViewer::HeaderStyleUtil::resentFromList(message);
        headerObject.insert( QLatin1String( "resentfrom" ), StringUtil::emailAddrAsAnchor( resentFrom, StringUtil::DisplayFullAddress ) );
    }

    if ( message->headerByType( "Resent-To" ) ) {
        const QList<KMime::Types::Mailbox> resentTo = MessageViewer::HeaderStyleUtil::resentToList(message);
        headerObject.insert( QLatin1String( "resenttoi18n"), i18np("receiver was", "receivers were", resentTo.count()));
        headerObject.insert( QLatin1String( "resentto" ), StringUtil::emailAddrAsAnchor( resentTo, StringUtil::DisplayFullAddress ) );
    }

    if ( KMime::Headers::Base *organization = message->headerByType("Organization") )
        headerObject.insert( QLatin1String( "organization" ) , MessageViewer::HeaderStyleUtil::strToHtml(organization->asUnicodeString()) );

    if ( !style->vCardName().isEmpty() )
        headerObject.insert( QLatin1String( "vcardname" ) , style->vCardName() );

    if ( !style->collectionName().isEmpty() )
        headerObject.insert( QLatin1String( "collectionname" ) , style->collectionName() );

    if ( isPrinting ) {
        //provide a bit more left padding when printing
        //kolab/issue3254 (printed mail cut at the left side)
        //Use it just for testing if we are in printing mode
        headerObject.insert( QLatin1String( "isprinting" ) , i18n("Printing mode") );
    }

    // colors depend on if it is encapsulated or not
    QColor fontColor( Qt::white );
    QString linkColor = QLatin1String("white");
    const QColor activeColor = KColorScheme( QPalette::Active, KColorScheme::Selection ).background().color();
    QColor activeColorDark = activeColor.dark(130);
    // reverse colors for encapsulated
    if ( !style->isTopLevel() ) {
        activeColorDark = activeColor.dark(50);
        fontColor = QColor(Qt::black);
        linkColor = QLatin1String("black");
    }

    // 3D borders
    headerObject.insert( QLatin1String( "activecolordark" ), activeColorDark.name() );
    headerObject.insert( QLatin1String( "fontcolor" ), fontColor.name() );
    headerObject.insert( QLatin1String( "linkcolor" ) , linkColor );


    MessageViewer::HeaderStyleUtil::xfaceSettings xface = MessageViewer::HeaderStyleUtil::xface(style, message);
    if ( !xface.photoURL.isEmpty() ) {
        headerObject.insert( QLatin1String( "photowidth" ) , xface.photoWidth );
        headerObject.insert( QLatin1String( "photoheight" ) , xface.photoHeight );
        headerObject.insert( QLatin1String( "photourl" ) , xface.photoURL );
    }

    Q_FOREACH (QString header, displayExtraHeaders) {
        const QByteArray baHeader = header.toLocal8Bit();
        if (message->headerByType(baHeader) ) {
            //Grantlee doesn't support '-' in variable name => remove it.
            header = header.remove(QLatin1Char('-'));
            headerObject.insert( header , message->headerByType(baHeader)->asUnicodeString() );
        }
    }

    headerObject.insert( QLatin1String( "vcardi18n" ), i18n("[vcard]") );
    const bool messageHasAttachment = KMime::hasAttachment(message);
    headerObject.insert( QLatin1String( "hasAttachment" ), messageHasAttachment);

    if (messageHasAttachment) {
        const QString iconPath = IconNameCache::instance()->iconPath(QLatin1String("mail-attachment"), KIconLoader::Toolbar);
        const QString html = QString::fromLatin1("<img height=\"22\" width=\"22\" src=\"file:///%1\"></a>").arg(iconPath);
        headerObject.insert( QLatin1String( "attachmentIcon" ), html);
    }

    QVariantHash mapping;
    mapping.insert( QLatin1String("header"), headerObject );
    Grantlee::Context context( mapping );

    return headerTemplate->render(&context);
}

}
Q_DECLARE_METATYPE(KMime::Headers::Cc*)
Q_DECLARE_METATYPE(KMime::Headers::To*)
Q_DECLARE_METATYPE(KMime::Headers::Bcc*)
