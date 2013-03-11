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


#include "grantleeheaderstyle.h"
#include "headerstyle/headerstyle_util.h"
#include "headerstyle/grantleeheaderformatter.h"

#include "headerstrategy.h"
#include <kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;
#include "globalsettings.h"

#include <kpimutils/email.h>
#include "kxface.h"
#include <messagecore/stringutil.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include <QApplication>

#include <kstandarddirs.h>
#include <KApplication>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;
using KPIMUtils::LinkLocator;
using namespace MessageViewer;

namespace MessageViewer {

GrantleeHeaderStyle::GrantleeHeaderStyle()
    : HeaderStyle()
{
    mGrantleeFormatter = new GrantleeHeaderFormatter;
}

GrantleeHeaderStyle::~GrantleeHeaderStyle()
{
    delete mGrantleeFormatter;
}


//
// GrantleeHeaderStyle:
//   show every header field on a line by itself,
//   show subject larger
//
QString GrantleeHeaderStyle::format( KMime::Message *message ) const {
    if ( !message )
        return QString();
    const HeaderStrategy *strategy = headerStrategy();
    if ( !strategy )
        strategy = HeaderStrategy::custom();

    QString themeName;//TODO load from settings.
    return mGrantleeFormatter->toHtml(themeName, isPrinting(), strategy, message);
}

QString GrantleeHeaderStyle::formatAllMessageHeaders( KMime::Message *message, const QStringList &headersToHide ) const
{
    QByteArray head = message->head();
    KMime::Headers::Base *header = KMime::HeaderParsing::extractFirstHeader( head );
    QString result;
    while ( header ) {
        const QString headerType = QLatin1String(header->type());
        if (!headersToHide.contains(headerType) || !headersToHide.contains(headerType.toLower())) {

            result += MessageViewer::HeaderStyleUtil::strToHtml(headerType) + QLatin1String(": ") + header->asUnicodeString();
            result += QLatin1String( "<br />\n" );
        }
        delete header;
        header = KMime::HeaderParsing::extractFirstHeader( head );
    }
    return result;
}
}
