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

#include "entrepriseheaderstyle.h"
#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;

#include <kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <kdebug.h>
#include <klocale.h>
#include <KColorScheme>
#include <KStandardDirs>


#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;
using KPIMUtils::LinkLocator;
using namespace MessageViewer;

namespace MessageViewer {

QString EnterpriseHeaderStyle::format( KMime::Message *message ) const
{
    if ( !message )
        return QString();
    const HeaderStrategy *strategy = headerStrategy();
    if ( !strategy ) {
        strategy = HeaderStrategy::brief();
    }

    // The direction of the header is determined according to the direction
    // of the application layout.
    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    // colors depend on if it is encapsulated or not
    QColor fontColor( Qt::white );
    QString linkColor = "class =\"white\"";
    const QColor activeColor = KColorScheme( QPalette::Active, KColorScheme::Selection ).
            background().color();
    QColor activeColorDark = activeColor.dark(130);
    // reverse colors for encapsulated
    if( !isTopLevel() ){
        activeColorDark = activeColor.dark(50);
        fontColor = QColor(Qt::black);
        linkColor = "class =\"black\"";
    }

    QString imgpath( KStandardDirs::locate("data","libmessageviewer/pics/") );
    imgpath.prepend( "file:///" );
    imgpath.append("enterprise_");
    const QString borderSettings( " padding-top: 0px; padding-bottom: 0px; border-width: 0px " );
    QString headerStr;

    // 3D borders
    if(isTopLevel())
        headerStr +=
                "<div style=\"position: fixed; top: 0px; left: 0px; background-color: #606060; "
                "width: 10px; min-height: 100%;\">&nbsp;</div>"
                "<div style=\"position: fixed; top: 0px; right: 0px;  background-color: #606060; "
                "width: 10px; min-height: 100%;\">&nbsp;</div>";

    headerStr +=
            "<div style=\"margin-left: 10px; top: 0px;\"><span style=\"font-size: 10px; font-weight: bold;\">"
            + MessageViewer::HeaderStyleUtil::dateString( message, isPrinting(), /* shortDate */ false ) + "</span></div>"
            // #0057ae
            "<table style=\"background: "+activeColorDark.name()+"; border-collapse:collapse; top: 14px; min-width: 200px; \" cellpadding=0> \n"
            "  <tr> \n"
            "   <td style=\"min-width: 6px; background-image: url("+imgpath+"top_left.png); \"></td> \n"
            "   <td style=\"height: 6px; width: 100%; background: url("+imgpath+"top.png); \"></td> \n"
            "   <td style=\"min-width: 6px; background: url("+imgpath+"top_right.png); \"></td> </tr> \n"
            "   <tr> \n"
            "   <td style=\"min-width: 6px; max-width: 6px; background: url("+imgpath+"left.png); \"></td> \n"
            "   <td style=\"\"> \n";
    headerStr +=
            "<div class=\"noprint\" style=\"z-index: 1; float:right; position: relative; top: -35px; right: 20px ; max-height: 65px\">\n"
            "<img src=\"" + imgpath + "icon.png\">\n"
            "</div>\n";
    headerStr +=
            "    <table style=\"color: "+fontColor.name()+" ! important; margin: 1px; border-spacing: 0px;\" cellpadding=0> \n";

    // subject
    if ( strategy->showHeader( "subject" ) ) {
        headerStr +=
                "     <tr> \n"
                "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; "+borderSettings+"\"></td> \n"
                "      <td style=\"font-weight: bolder; font-size: 120%; padding-right: 91px; "+borderSettings+"\">";
        headerStr += MessageViewer::HeaderStyleUtil::subjectString( message )+ "</td> \n"
                "     </tr> \n";
    }

    // from
    if ( strategy->showHeader( "from" ) ) {
        // TODO vcard
        // We by design use the stripped mail address here, it is more enterprise-like.
        QString fromPart = StringUtil::emailAddrAsAnchor( message->from(),
                                                          StringUtil::DisplayNameOnly, linkColor );
        if ( !vCardName().isEmpty() )
            fromPart += "&nbsp;&nbsp;<a href=\"" + vCardName() + "\" "+linkColor+">" + i18n("[vCard]") + "</a>";
        //TDDO strategy date
        //if ( strategy->showHeader( "date" ) )
        headerStr +=
                "     <tr> \n"
                "      <td style=\"font-size: 10px; padding-left: 5px; padding-right: 24px; text-align: right; vertical-align:top; "+borderSettings+"\">"+i18n("From: ")+"</td> \n"
                "      <td style=\""+borderSettings+"\">"+ fromPart +"</td> "
                "     </tr> ";
    }

    // to line
    if ( strategy->showHeader( "to" ) ) {
        headerStr +=
                "     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("To: ") + "</td> "
                "      <td style=\"" + borderSettings + "\">" +
                StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor ) +
                "      </td> "
                "     </tr>\n";
    }

    // cc line, if any
    if ( strategy->showHeader( "cc" ) && message->cc( false ) ) {
        headerStr +=
                "     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("CC: ") + "</td> "
                "      <td style=\"" + borderSettings + "\">" +
                StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor ) +
                "      </td> "
                "     </tr>\n";
    }

    // bcc line, if any
    if ( strategy->showHeader( "bcc" ) && message->bcc( false ) ) {
        headerStr +=
                "     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("BCC: ") + "</td> "
                "      <td style=\"" + borderSettings + "\">" +
                StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress, linkColor ) +
                "      </td> "
                "     </tr>\n";
    }

    // attachments
    QString attachment;
    if( isTopLevel() ) {
        attachment =
                "<tr>"
                "<td style='min-width: 6px; max-width: 6px; background: url("+imgpath+"left.png);'</td>"
                "<td style='padding-right:20px;'>"
                "<div class=\"noprint\" >"
                "<div id=\"attachmentInjectionPoint\"></div>"
                "</div>"
                "</td>"
                "<td style='min-width: 6px; max-width: 6px; background: url("+imgpath+"right.png);'</td>"
                "</tr>";
    }

    // header-bottom
    headerStr +=
            "    </table> \n"
            "   </td> \n"
            "   <td style=\"min-width: 6px; max-height: 15px; background: url("+imgpath+"right.png); \"></td> \n"
            "  </tr> \n" + attachment +
            "  <tr> \n"
            "   <td style=\"min-width: 6px; background: url("+imgpath+"s_left.png); \"></td> \n"
            "   <td style=\"height: 35px; width: 80%; background: url("+imgpath+"sbar.png);\"> \n"
            "    <img src=\""+imgpath+"sw.png\" style=\"margin: 0px; height: 30px; overflow:hidden; \"> \n"
            "    <img src=\""+imgpath+"sp_right.png\" style=\"float: right; \"> </td> \n"
            "   <td style=\"min-width: 6px; background: url("+imgpath+"s_right.png); \"></td> \n"
            "  </tr> \n"
            " </table> \n";

    if ( isPrinting() ) {
        //provide a bit more left padding when printing
        //kolab/issue3254 (printed mail cut at the left side)
        headerStr += "<div style=\"padding: 6px; padding-left: 10px;\">";
    } else {
        headerStr += "<div style=\"padding: 6px;\">";
    }

    // TODO
    // spam status
    // ### iterate over the rest of strategy->headerToDisplay() (or
    // ### all headers if DefaultPolicy == Display) (elsewhere, too)
    return headerStr;
}
}
