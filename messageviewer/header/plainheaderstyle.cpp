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

#include "plainheaderstyle.h"

#include "header/headerstyle_util.h"


#include "header/headerstrategy.h"

#include <kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <kdebug.h>
#include <KLocale>
#include <KApplication>

#include <kmime/kmime_message.h>

using namespace MessageCore;

namespace MessageViewer {
//
// PlainHeaderStyle:
//   show every header field on a line by itself,
//   show subject larger
//
QString PlainHeaderStyle::format( KMime::Message *message ) const {
    if ( !message )
        return QString();
    const HeaderStrategy *strategy = headerStrategy();
    if ( !strategy )
        strategy = HeaderStrategy::rich();

    // The direction of the header is determined according to the direction
    // of the application layout.

    QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    const QString subjectDir = MessageViewer::HeaderStyleUtil::subjectDirectionString( message );
    QString headerStr;

    if ( strategy->headersToDisplay().isEmpty()
         && strategy->defaultPolicy() == HeaderStrategy::Display ) {
        // crude way to emulate "all" headers - Note: no strings have
        // i18n(), so direction should always be ltr.
        headerStr= QString("<div class=\"header\" dir=\"ltr\">");
        headerStr += formatAllMessageHeaders( message );
        return headerStr + "</div>";
    }

    headerStr = QString("<div class=\"header\" dir=\"%1\">").arg(dir);

    //case HdrLong:
    if ( strategy->showHeader( "subject" ) )
        headerStr += QString("<div dir=\"%1\"><b style=\"font-size:130%\">" +
                                         MessageViewer::HeaderStyleUtil::subjectString( message ) + "</b></div>\n")
                .arg(subjectDir);

    if ( strategy->showHeader( "date" ) )
        headerStr.append(i18n("Date: ") + MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting(), /* short = */ false ) ) + "<br/>\n" );

    if ( strategy->showHeader( "from" ) ) {
        /*FIXME(Andras) review if it is still needed
    if ( fromStr.isEmpty() ) // no valid email in from, maybe just a name
      fromStr = message->fromStrip(); // let's use that
*/
        headerStr.append( i18n("From: ") +
                          StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, "", StringUtil::ShowLink ) );
        if ( !vCardName().isEmpty() )
            headerStr.append("&nbsp;&nbsp;<a href=\"" + vCardName() +
                             "\">" + i18n("[vCard]") + "</a>" );

        if ( strategy->showHeader( "organization" )
             && message->headerByType("Organization"))
            headerStr.append("&nbsp;&nbsp;(" +
                             MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("Organization")->asUnicodeString()) + ')');
        headerStr.append("<br/>\n");
    }

    if ( strategy->showHeader( "to" ) )
        headerStr.append( i18nc("To-field of the mailheader.", "To: ") +
                          StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress ) + "<br/>\n" );

    if ( strategy->showHeader( "cc" ) && message->cc( false ) )
        headerStr.append( i18n("CC: ") +
                          StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ) + "<br/>\n" );

    if ( strategy->showHeader( "bcc" ) && message->bcc( false ) )
        headerStr.append( i18n("BCC: ") +
                          StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ) + "<br/>\n" );

    if ( strategy->showHeader( "reply-to" ) && message->replyTo( false ) )
        headerStr.append( i18n("Reply to: ") +
                          StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ) + "<br/>\n" );

    headerStr += "</div>\n";

    return headerStr;
}

QString PlainHeaderStyle::formatAllMessageHeaders( KMime::Message *message ) const {
    QByteArray head = message->head();
    KMime::Headers::Base *header = KMime::HeaderParsing::extractFirstHeader( head );
    QString result;
    while ( header ) {
        result += MessageViewer::HeaderStyleUtil::strToHtml( QLatin1String(header->type()) + QLatin1String(": ") + header->asUnicodeString() );
        result += QLatin1String( "<br />\n" );
        delete header;
        header = KMime::HeaderParsing::extractFirstHeader( head );
    }

    return result;
}
}
