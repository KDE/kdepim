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

#include "entrepriseheaderstyle.h"
#include "header/headerstyle_util.h"

#include "header/headerstrategy.h"
#include <KPIMUtils/kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;

#include <KPIMUtils/kpimutils/email.h>
#include <messagecore/utils/stringutil.h>

#include <qdebug.h>
#include <KLocalizedString>
#include <KColorScheme>



#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>
#include <QStandardPaths>

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
    QString linkColor = QLatin1String("class =\"white\"");
    const QColor activeColor = KColorScheme( QPalette::Active, KColorScheme::Selection ).
            background().color();
    QColor activeColorDark = activeColor.dark(130);
    // reverse colors for encapsulated
    if( !isTopLevel() ){
        activeColorDark = activeColor.dark(50);
        fontColor = QColor(Qt::black);
        linkColor = QLatin1String("class =\"black\"");
    }

    QString imgpath( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("libmessageviewer/pics/")) );
    imgpath.prepend( QLatin1String("file:///") );
    imgpath.append(QLatin1String("enterprise_"));
    const QString borderSettings( QLatin1String(" padding-top: 0px; padding-bottom: 0px; border-width: 0px ") );
    QString headerStr;

    // 3D borders
    if(isTopLevel())
        headerStr +=
                QLatin1String("<div style=\"position: fixed; top: 0px; left: 0px; background-color: #606060; "
                "width: 10px; min-height: 100%;\">&nbsp;</div>"
                "<div style=\"position: fixed; top: 0px; right: 0px;  background-color: #606060; "
                "width: 10px; min-height: 100%;\">&nbsp;</div>");

    headerStr +=
            QLatin1String("<div style=\"margin-left: 10px; top: 0px;\"><span style=\"font-size: 10px; font-weight: bold;\">")
            + MessageViewer::HeaderStyleUtil::dateString( message, isPrinting(), /* shortDate */ false ) + QLatin1String("</span></div>"
            // #0057ae
            "<table style=\"background: ")+activeColorDark.name()+QLatin1String("; border-collapse:collapse; top: 14px; min-width: 200px; \" cellpadding=0> \n"
            "  <tr> \n"
            "   <td style=\"min-width: 6px; background-image: url(")+imgpath+QLatin1String("top_left.png); \"></td> \n"
            "   <td style=\"height: 6px; width: 100%; background: url(")+imgpath+QLatin1String("top.png); \"></td> \n"
            "   <td style=\"min-width: 6px; background: url(")+imgpath+QLatin1String("top_right.png); \"></td> </tr> \n"
            "   <tr> \n"
            "   <td style=\"min-width: 6px; max-width: 6px; background: url(")+imgpath+QLatin1String("left.png); \"></td> \n"
            "   <td style=\"\"> \n");
    headerStr +=
            QLatin1String("<div class=\"noprint\" style=\"z-index: 1; float:right; position: relative; top: -35px; right: 20px ; max-height: 65px\">\n"
            "<img src=\"") + imgpath + QLatin1String("icon.png\">\n"
            "</div>\n");
    headerStr +=
            QLatin1String("    <table style=\"color: ")+fontColor.name()+QLatin1String(" ! important; margin: 1px; border-spacing: 0px;\" cellpadding=0> \n");

    // subject
    if ( strategy->showHeader( QLatin1String("subject") ) ) {
        headerStr +=
                QLatin1String("     <tr> \n"
                "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; ")+borderSettings+QLatin1String("\"></td> \n"
                "      <td style=\"font-weight: bolder; font-size: 120%; padding-right: 91px; ")+borderSettings+QLatin1String("\">");
        headerStr += MessageViewer::HeaderStyleUtil::subjectString( message )+ QLatin1String("</td> \n"
                "     </tr> \n");
    }

    // from
    if ( strategy->showHeader( QLatin1String("from") ) ) {
        // We by design use the stripped mail address here, it is more enterprise-like.
        QString fromPart = StringUtil::emailAddrAsAnchor( message->from(),
                                                          StringUtil::DisplayNameOnly, linkColor );
        if ( !vCardName().isEmpty() )
            fromPart += QLatin1String("&nbsp;&nbsp;<a href=\"") + vCardName() + QLatin1String("\" ")+linkColor+ QLatin1Char('>') + i18n("[vCard]") + QLatin1String("</a>");
        //TDDO strategy date
        //if ( strategy->showHeader( "date" ) )
        headerStr +=
                QLatin1String("     <tr> \n"
                "      <td style=\"font-size: 10px; padding-left: 5px; padding-right: 24px; text-align: right; vertical-align:top; ")+borderSettings+QLatin1String("\">")+i18n("From: ")+QLatin1String("</td> \n"
                "      <td style=\"")+borderSettings+QLatin1String("\">")+ fromPart +QLatin1String("</td> "
                "     </tr> ");
    }

    // to line
    if ( strategy->showHeader( QLatin1String("to") ) ) {
        headerStr +=
                QLatin1String("     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; ") + borderSettings + QLatin1String("\">") + i18n("To: ") + QLatin1String("</td> "
                "      <td style=\"") + borderSettings + QLatin1String("\">") +
                StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor ) +
                QLatin1String("      </td> "
                "     </tr>\n");
    }

    // cc line, if any
    if ( strategy->showHeader( QLatin1String("cc") ) && message->cc( false ) ) {
        headerStr +=
                QLatin1String("     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; ") + borderSettings + QLatin1String("\">") + i18n("CC: ") + QLatin1String("</td> "
                "      <td style=\"") + borderSettings + QLatin1String("\">") +
                StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor ) +
                QLatin1String("      </td> "
                "     </tr>\n");
    }

    // bcc line, if any
    if ( strategy->showHeader( QLatin1String("bcc") ) && message->bcc( false ) ) {
        headerStr +=
                QLatin1String("     <tr> "
                "      <td style=\"font-size: 10px; text-align: right; vertical-align:top; padding-left: 5px; padding-right: 24px; ") + borderSettings + QLatin1String("\">") + i18n("BCC: ") + QLatin1String("</td> "
                "      <td style=\"") + borderSettings + QLatin1String("\">") +
                StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress, linkColor ) +
                QLatin1String("      </td> "
                "     </tr>\n");
    }

    // attachments
    QString attachment;
    if( isTopLevel() ) {
        attachment =
                QLatin1String("<tr>"
                "<td style='min-width: 6px; max-width: 6px; background: url(")+imgpath+QLatin1String("left.png);'</td>"
                "<td style='padding-right:20px;'>"
                "<div class=\"noprint\" >"
                "<div id=\"attachmentInjectionPoint\"></div>"
                "</div>"
                "</td>"
                "<td style='min-width: 6px; max-width: 6px; background: url(")+imgpath+QLatin1String("right.png);'</td>"
                "</tr>");
    }

    // header-bottom
    headerStr +=
            QLatin1String("    </table> \n"
            "   </td> \n"
            "   <td style=\"min-width: 6px; max-height: 15px; background: url(")+imgpath+QLatin1String("right.png); \"></td> \n"
            "  </tr> \n") + attachment +
            QLatin1String("  <tr> \n"
            "   <td style=\"min-width: 6px; background: url(")+imgpath+QLatin1String("s_left.png); \"></td> \n"
            "   <td style=\"height: 35px; width: 80%; background: url(")+imgpath+QLatin1String("sbar.png);\"> \n"
            "    <img src=\"")+imgpath+QLatin1String("sw.png\" style=\"margin: 0px; height: 30px; overflow:hidden; \"> \n"
            "    <img src=\"")+imgpath+QLatin1String("sp_right.png\" style=\"float: right; \"> </td> \n"
            "   <td style=\"min-width: 6px; background: url(")+imgpath+QLatin1String("s_right.png); \"></td> \n"
            "  </tr> \n"
            " </table> \n");

    if ( isPrinting() ) {
        //provide a bit more left padding when printing
        //kolab/issue3254 (printed mail cut at the left side)
        headerStr += QLatin1String("<div style=\"padding: 6px; padding-left: 10px;\">");
    } else {
        headerStr += QLatin1String("<div style=\"padding: 6px;\">");
    }

    // TODO
    // spam status
    // ### iterate over the rest of strategy->headerToDisplay() (or
    // ### all headers if DefaultPolicy == Display) (elsewhere, too)
    return headerStr;
}
}
