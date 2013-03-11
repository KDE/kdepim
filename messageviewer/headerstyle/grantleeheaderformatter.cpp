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
#include "headerstrategy.h"
#include "headerstyle_util.h"

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
        templateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
        templateLoader->setTemplateDirs( QStringList() << KStandardDirs::locate("data",QLatin1String("messageviewer/themes/")) );
        templateLoader->setTheme( QLatin1String( "default" ) );
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

QString GrantleeHeaderFormatter::toHtml(const QString &themeName, const MessageViewer::HeaderStrategy *strategy, KMime::Message *message) const
{
    Grantlee::Template headerTemplate = d->engine->loadByName( themeName + "/default.html" );
    QString errorMessage;
    if ( headerTemplate->error() ) {
      errorMessage += headerTemplate->errorString();
    }
    if ( !errorMessage.isEmpty() ) {
      return errorMessage;
    }

    QVariantHash headerObject;
    if ( strategy->showHeader( "subject" ) ) {
        headerObject.insert(QLatin1String("subjecti18n"), i18n("Subject:") );
        headerObject.insert(QLatin1String("subject"), MessageViewer::HeaderStyleUtil::subjectString( message ) );
    }

    if ( strategy->showHeader("replyto") && message->replyTo( false )) {
        headerObject.insert(QLatin1String("replyToi18n"), i18n("Reply to:") );
        headerObject.insert(QLatin1String("replyTo"), StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ));
    }

    if ( strategy->showHeader( "cc" ) && message->cc( false ) ) {
        headerObject.insert(QLatin1String("cci18n"), i18n("CC:") );
        headerObject.insert(QLatin1String("cc"), StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ));
    }

    if ( strategy->showHeader( "bcc" ) && message->bcc( false ) ) {
        headerObject.insert(QLatin1String("bcci18n"), i18n("BCC:"));
        headerObject.insert(QLatin1String("bcc"), StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ));
    }


    QVariantHash mapping;
    mapping.insert( "header", headerObject );
    Grantlee::Context context( mapping );

    return headerTemplate->render(&context);
}

}
