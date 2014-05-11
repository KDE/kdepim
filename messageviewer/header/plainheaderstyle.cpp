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

#include "plainheaderstyle.h"

#include "header/headerstyle_util.h"


#include "header/headerstrategy.h"

#include <KPIMUtils/kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <qdebug.h>
#include <KLocalizedString>
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

    QString dir = QApplication::isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr");

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
        headerStr= QLatin1String("<div class=\"header\" dir=\"ltr\">");
        headerStr += formatAllMessageHeaders( message );
        return headerStr + QLatin1String("</div>");
    }

    headerStr = QString::fromLatin1("<div class=\"header\" dir=\"%1\">").arg(dir);

    //case HdrLong:
    if ( strategy->showHeader( QLatin1String("subject") ) )
        headerStr += QString::fromLatin1("<div dir=\"%1\"><b style=\"font-size:130%\">").arg(subjectDir) +
                                         MessageViewer::HeaderStyleUtil::subjectString( message ) + QLatin1String("</b></div>\n");

    if ( strategy->showHeader( QLatin1String("date") ) )
        headerStr.append(i18n("Date: ") + MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString(message, isPrinting(), /* short = */ false ) ) + QLatin1String("<br/>\n") );

    if ( strategy->showHeader( QLatin1String("from") ) ) {
        /*FIXME(Andras) review if it is still needed
    if ( fromStr.isEmpty() ) // no valid email in from, maybe just a name
      fromStr = message->fromStrip(); // let's use that
*/
        headerStr.append( i18n("From: ") +
                          StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, QString(), StringUtil::ShowLink ) );
        if ( !vCardName().isEmpty() )
            headerStr.append(QLatin1String("&nbsp;&nbsp;<a href=\"") + vCardName() +
                             QLatin1String("\">") + i18n("[vCard]") + QLatin1String("</a>") );

        if ( strategy->showHeader( QLatin1String("organization") )
             && message->headerByType("Organization"))
            headerStr.append(QLatin1String("&nbsp;&nbsp;(") +
                             MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("Organization")->asUnicodeString()) + QLatin1Char(')'));
        headerStr.append(QLatin1String("<br/>\n"));
    }

    if ( strategy->showHeader( QLatin1String("to") ) )
        headerStr.append( i18nc("To-field of the mailheader.", "To: ") +
                          StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress ) + QLatin1String("<br/>\n") );

    if ( strategy->showHeader( QLatin1String("cc") ) && message->cc( false ) )
        headerStr.append( i18n("CC: ") +
                          StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ) + QLatin1String("<br/>\n") );

    if ( strategy->showHeader( QLatin1String("bcc") ) && message->bcc( false ) )
        headerStr.append( i18n("BCC: ") +
                          StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ) + QLatin1String("<br/>\n") );

    if ( strategy->showHeader( QLatin1String("reply-to" )) && message->replyTo( false ) )
        headerStr.append( i18n("Reply to: ") +
                          StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ) + QLatin1String("<br/>\n") );

    headerStr += QLatin1String("</div>\n");

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
