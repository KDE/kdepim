/*  -*- c++ -*-
    header/headerstyle.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    Copyright (c) 2013 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "briefheaderstyle.h"
#include "header/headerstyle.h"
#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <KPIMUtils/kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;

#include <KPIMUtils/kpimutils/email.h>
#include <messagecore/utils/stringutil.h>
#include <qdebug.h>
#include <KLocalizedString>


#include <KDateTime>
#include <QRegExp>

#include <QApplication>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;
using KPIMUtils::LinkLocator;
using namespace MessageViewer;
//
// BriefHeaderStyle
//   Show everything in a single line, don't show header field names.
//

QString BriefHeaderStyle::format( KMime::Message *message ) const {
    if ( !message ) return QString();

    const HeaderStrategy *strategy = headerStrategy();
    if ( !strategy )
        strategy = HeaderStrategy::brief();

    // The direction of the header is determined according to the direction
    // of the application layout.

    const QString dir = QApplication::isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr");

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    const QString subjectDir = MessageViewer::HeaderStyleUtil::subjectDirectionString( message );

    QString headerStr = QLatin1String("<div class=\"header\" dir=\"") + dir + QLatin1String("\">\n");

    if ( strategy->showHeader( QLatin1String("subject") ) ) {
        headerStr += QLatin1String("<div dir=\"") + subjectDir + QLatin1String("\">\n") +
                QLatin1String("<b style=\"font-size:130%\">");

        headerStr += MessageViewer::HeaderStyleUtil::subjectString( message ) + QLatin1String("</b></div>\n");
    }
    QStringList headerParts;

    if ( strategy->showHeader( QLatin1String("from") ) ) {
        /*TODO(Andras) review if it can happen or not
    if ( fromStr.isEmpty() ) // no valid email in from, maybe just a name
      fromStr = message->fromStrip(); // let's use that
*/
        QString fromPart = StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly );
        if ( !vCardName().isEmpty() )
            fromPart += QLatin1String("&nbsp;&nbsp;<a href=\"") + vCardName() + QLatin1String("\">") + i18n("[vCard]") + QLatin1String("</a>");
        headerParts << fromPart;
    }

    if ( strategy->showHeader( QLatin1String("cc") ) && message->cc(false) )
        headerParts << i18n("CC: ") + StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayNameOnly );

    if ( strategy->showHeader( QLatin1String("bcc") ) && message->bcc(false) )
        headerParts << i18n("BCC: ") + StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayNameOnly );

    if ( strategy->showHeader( QLatin1String("date") ) )
        headerParts << MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString( message, isPrinting(), /* shortDate = */ true ) );

    // remove all empty (modulo whitespace) entries and joins them via ", \n"
    headerStr += QLatin1String(" (") + headerParts.filter( QRegExp( QLatin1String("\\S") ) ).join( QLatin1String(",\n") ) + QLatin1Char(')');

    headerStr += QLatin1String("</div>\n");

    // ### iterate over the rest of strategy->headerToDisplay() (or
    // ### all headers if DefaultPolicy == Display) (elsewhere, too)
    return headerStr;
}
