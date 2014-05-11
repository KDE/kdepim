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

#include "mobileheaderstyle.h"

#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <KPIMUtils/kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;

#include <KPIMUtils/kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <qdebug.h>
#include <klocale.h>

#include <QFontMetrics>

#include <kstandarddirs.h>
#include <KApplication>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;
using KPIMUtils::LinkLocator;
using namespace MessageViewer;

namespace MessageViewer {

static int matchingFontSize( const QString &text, int maximumWidth, int fontPixelSize )
{
    int pixelSize = fontPixelSize;
    while ( true ) {
        if ( pixelSize <= 8 )
            break;

        QFont font;
        font.setPixelSize( pixelSize );
        QFontMetrics fm( font );
        if ( fm.width( text ) <= maximumWidth )
            break;

        pixelSize--;
    }

    return pixelSize;
}

static QString formatMobileHeader( KMime::Message *message, bool extendedFormat, const HeaderStyle *style )
{
    if ( !message )
        return QString();

    // From
    QString linkColor =QLatin1String("style=\"color: #0E49A1; text-decoration: none\"");
    QString fromPart = StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, linkColor );

    if ( !style->vCardName().isEmpty() )
        fromPart += QLatin1String("&nbsp;&nbsp;<a href=\"") + style->vCardName() + QLatin1String("\" ") + linkColor + QLatin1Char('>') + i18n( "[vCard]" ) + QLatin1String("</a>");

    const QString toPart = StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor );
    const QString ccPart = StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor );

    // Background image
    const QString imagePath( QLatin1String( "file:///" ) + KStandardDirs::locate( "data", QLatin1String("libmessageviewer/pics/") ) );
    const QString mobileImagePath( imagePath + QLatin1String( "mobile_" ) );
    //const QString mobileExtendedImagePath( imagePath + QLatin1String( "mobileextended_" ) );

    const Akonadi::MessageStatus status = style->messageStatus();
    QString flagsPart;
    if ( status.isImportant() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_important.png\" height=\"22\" width=\"22\"/>");
    if ( status.hasAttachment() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_attachment.png\" height=\"22\" width=\"22\"/>");
    if ( status.isToAct() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_actionitem.png\" height=\"22\" width=\"22\"/>");
    if ( status.isReplied() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_replied.png\" height=\"22\" width=\"22\"/>");
    if ( status.isForwarded() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_forwarded.png\" height=\"22\" width=\"22\"/>");
    if ( status.isSigned() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_signed.png\" height=\"22\" width=\"22\"/>");
    if ( status.isEncrypted() )
        flagsPart += QLatin1String("<img src=\"") + mobileImagePath + QLatin1String("status_encrypted.png\" height=\"22\" width=\"22\"/>");


    QString headerStr;
    headerStr += QLatin1String("<div style=\"width: 100%\">\n");
    headerStr += QLatin1String("  <table width=\"100%\" bgcolor=\"#B4E3F7\">\n");
    headerStr += QLatin1String("    <tr>\n");
    headerStr += QLatin1String("      <td valign=\"bottom\" width=\"80%\">\n");
    headerStr += QLatin1String("        <div style=\"text-align: left; font-size: 20px; color: #0E49A1\">") + fromPart + QLatin1String("</div>\n");
    headerStr += QLatin1String("      </td>\n");
    headerStr += QLatin1String("      <td valign=\"bottom\" width=\"20%\" align=\"right\">\n");
    headerStr += QLatin1String("        <div style=\"text-align: right; color: #0E49A1;\">") + flagsPart + QLatin1String("</div>\n");
    headerStr += QLatin1String("      </td>\n");
    headerStr += QLatin1String("    </tr>\n");
    headerStr += QLatin1String("  </table>\n");
    headerStr += QLatin1String("  <table width=\"100%\" bgcolor=\"#B4E3F7\">\n");
    if ( extendedFormat ) {
        headerStr += QLatin1String("    <tr>\n");
        headerStr += QLatin1String("      <td valign=\"bottom\" colspan=\"2\">\n");
        headerStr += QLatin1String("        <div style=\"height: 20px; font-size: 15px; color: #0E49A1\">") + toPart + ccPart + QLatin1String("</div>\n");
        headerStr += QLatin1String("      </td>\n");
        headerStr += QLatin1String("    </tr>\n");
    }

    const int subjectFontSize = matchingFontSize( message->subject()->asUnicodeString(), 650, 20 );
    headerStr += QLatin1String("    <tr>\n");
    headerStr += QLatin1String("      <td valign=\"bottom\" colspan=\"2\">\n");
    headerStr += QLatin1String("        <div style=\"height: 35px; font-size: ") + QString::number( subjectFontSize ) + QLatin1String("px; color: #24353F;\">") + message->subject()->asUnicodeString() + QLatin1String("</div>\n");
    headerStr += QLatin1String("      </td>\n");
    headerStr += QLatin1String("    </tr>\n");
    headerStr += QLatin1String("    <tr>\n");
    headerStr += QLatin1String("      <td align=\"left\" width=\"50%\">\n");
    if ( !style->messagePath().isEmpty() ) {
        headerStr += QLatin1String("        <div style=\"font-size: 15px; color: #24353F\">") + style->messagePath() + QLatin1String("</div>\n");
    }
    headerStr += QLatin1String("      </td>\n");
    headerStr += QLatin1String("      <td align=\"right\" width=\"50%\">\n");
    headerStr += QLatin1String("        <div style=\"font-size: 15px; color: #24353F; text-align: right; margin-right: 15px\">") + i18n( "sent: " );
    headerStr += MessageViewer::HeaderStyleUtil::dateString( message, style->isPrinting(), /* shortDate = */ false ) + QLatin1String("</div>\n");
    headerStr += QLatin1String("      </td>\n");
    headerStr += QLatin1String("    </tr>\n");
    headerStr += QLatin1String("  </table>\n");
    headerStr += QLatin1String("  <br/>\n");
    headerStr += QLatin1String("</div>\n");

    return headerStr;
}

QString MobileHeaderStyle::format( KMime::Message *message ) const
{
    return formatMobileHeader( message, false, this );
}

QString MobileExtendedHeaderStyle::format( KMime::Message *message ) const
{
    return formatMobileHeader( message, true, this );
}
}
