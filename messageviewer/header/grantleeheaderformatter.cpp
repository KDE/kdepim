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

#include "grantleeheaderformatter.h"
#include "grantleeheaderstyle.h"
#include "headerstyle_util.h"
#include "globalsettings.h"
#include "config-messageviewer.h"
#include "contactdisplaymessagememento.h"
#include "kxface.h"

#include <kpimutils/email.h>
#include <messagecore/stringutil.h>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>


#include <KLocale>
#include <KStandardDirs>

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
        engine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH << MESSAGEVIEWER_GRANTLEE_PLUGIN_PATH);
        templateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
        templateLoader->setTemplateDirs( QStringList() << KStandardDirs::locate("data",QLatin1String("messageviewer/themes/")) );
        engine->addTemplateLoader( templateLoader );

    }
    ~Private()
    {
        delete engine;
    }

    QString templatePath;
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

QString GrantleeHeaderFormatter::toHtml(const QString &themeName, bool isPrinting, const MessageViewer::GrantleeHeaderStyle *style, KMime::Message *message) const
{
    Grantlee::Template headerTemplate = d->engine->loadByName( themeName + "/header.html" );
    QString errorMessage;
    if ( headerTemplate->error() ) {
        errorMessage += headerTemplate->errorString();
    }
    if ( !errorMessage.isEmpty() ) {
        return errorMessage;
    }

    QVariantHash headerObject;
    headerObject.insert(QLatin1String("subjecti18n"), i18n("Subject:") );
    headerObject.insert(QLatin1String("subject"), MessageViewer::HeaderStyleUtil::subjectString( message ) );

    if ( message->replyTo( false )) {
        headerObject.insert(QLatin1String("replyToi18n"), i18n("Reply to:") );
        headerObject.insert(QLatin1String("replyTo"), StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("replyToStr"), message->replyTo()->asUnicodeString());
    }

    if ( message->cc( false ) ) {
        headerObject.insert(QLatin1String("cci18n"), i18n("CC:") );
        headerObject.insert(QLatin1String("cc"), StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("ccStr"), message->cc()->asUnicodeString());
    }

    if ( message->bcc( false ) ) {
        headerObject.insert(QLatin1String("bcci18n"), i18n("BCC:"));
        headerObject.insert(QLatin1String("bcc"), StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ));
        headerObject.insert(QLatin1String("bccStr"), message->bcc()->asUnicodeString());
    }
    headerObject.insert(QLatin1String("fromi18n"), i18n("From:"));
    headerObject.insert(QLatin1String( "from" ) ,  StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress ) );
    headerObject.insert(QLatin1String( "fromStr" ) , message->from()->asUnicodeString() );


    const QString spamHtml = MessageViewer::HeaderStyleUtil::spamStatus(message);
    if ( !spamHtml.isEmpty() ) {
        headerObject.insert( QLatin1String( "spamHTML" ), spamHtml );
    }
    headerObject.insert(QLatin1String("datei18n"), i18n("Date:"));

    headerObject.insert( QLatin1String( "dateshort" ) , MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting,true ) ) );
    headerObject.insert( QLatin1String( "datelong" ) , MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting,false ) ) );
    headerObject.insert( QLatin1String( "date" ), MessageViewer::HeaderStyleUtil::directionOf( MessageViewer::HeaderStyleUtil::dateStr( message->date()->dateTime() ) ) );

    if ( GlobalSettings::self()->showUserAgent() ) {
        if ( message->headerByType("User-Agent") ) {
            headerObject.insert( QLatin1String( "useragent" ), MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("User-Agent")->as7BitString() ) );
        }

        if ( message->headerByType("X-Mailer") ) {
            headerObject.insert( QLatin1String( "x-mailer" ), MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("X-Mailer")->as7BitString() ) );
        }
    }

    if ( message->headerByType( "Resent-From" ) ) {
        const QList<KMime::Types::Mailbox> resentFrom = MessageViewer::HeaderStyleUtil::resentFromList(message);
        headerObject.insert( QLatin1String( "resentfrom" ), StringUtil::emailAddrAsAnchor( resentFrom, StringUtil::DisplayFullAddress ) );
    }

    if ( KMime::Headers::Base *organization = message->headerByType("Organization") )
        headerObject.insert( QLatin1String( "organization" ) , MessageViewer::HeaderStyleUtil::strToHtml(organization->asUnicodeString()) );

    if ( !style->vCardName().isEmpty() )
        headerObject.insert( QLatin1String( "vcardname" ) , style->vCardName() );


    // colors depend on if it is encapsulated or not
    QColor fontColor( Qt::white );
    QString linkColor = QLatin1String("white");
    const QColor activeColor = KColorScheme( QPalette::Active, KColorScheme::Selection ).background().color();
    QColor activeColorDark = activeColor.dark(130);
    // reverse colors for encapsulated
    if( !style->isTopLevel() ){
        activeColorDark = activeColor.dark(50);
        fontColor = QColor(Qt::black);
        linkColor = QLatin1String("black");
    }

    // 3D borders
    headerObject.insert( QLatin1String( "activecolordark" ), activeColorDark.name() );
    headerObject.insert( QLatin1String( "fontcolor" ), fontColor.name() );
    headerObject.insert( QLatin1String( "linkcolor" ) , linkColor );


    MessageViewer::HeaderStyleUtil::xfaceSettings xface = MessageViewer::HeaderStyleUtil::xface(style, message);
    if( !xface.photoURL.isEmpty() )
    {
        headerObject.insert( QLatin1String( "photowidth" ) , xface.photoWidth );
        headerObject.insert( QLatin1String( "photoheight" ) , xface.photoHeight );
        headerObject.insert( QLatin1String( "photourl" ) , xface.photoURL );
    }

    QVariantHash mapping;
    mapping.insert( "header", headerObject );
    Grantlee::Context context( mapping );

    return headerTemplate->render(&context);
}

}
