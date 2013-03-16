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

#include "fancyheaderstyle.h"

#include "headerstyle.h"
#include "header/headerstyle_util.h"


#include "headerstrategy.h"
#include <kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;
#include "globalsettings.h"
#include "nodehelper.h"
#include "contactdisplaymessagememento.h"

#include <kpimutils/email.h>
#include "kxface.h"
#include <messagecore/stringutil.h>

#include <akonadi/contact/contactsearchjob.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcodecs.h>
#include <KColorScheme>

#include <KDateTime>
#include <QBuffer>
#include <QBitmap>
#include <QImage>
#include <QApplication>
#include <QRegExp>
#include <QFontMetrics>

#include <kstandarddirs.h>
#include <KApplication>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;

namespace MessageViewer {
//
// FancyHeaderStyle:
//   Like PlainHeaderStyle, but with slick frames and background colours.
//

QString FancyHeaderStyle::format( KMime::Message *message ) const {
    if ( !message )
        return QString();
    const HeaderStrategy *strategy = headerStrategy();
    if ( !strategy )
        strategy = HeaderStrategy::rich();

    // ### from kmreaderwin begin
    // The direction of the header is determined according to the direction
    // of the application layout.

    const QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
    QString headerStr = QString::fromLatin1("<div class=\"fancy header\" dir=\"%1\">\n").arg(dir);

    // However, the direction of the message subject within the header is
    // determined according to the contents of the subject itself. Since
    // the "Re:" and "Fwd:" prefixes would always cause the subject to be
    // considered left-to-right, they are ignored when determining its
    // direction.

    const QString subjectDir = MessageViewer::HeaderStyleUtil::subjectDirectionString( message );

    // Spam header display.
    // If the spamSpamStatus config value is true then we look for headers
    // from a few spam filters and try to create visually meaningful graphics
    // out of the spam scores.

    const QString spamHTML =  MessageViewer::HeaderStyleUtil::spamStatus(message);

    QString photoURL;
    int photoWidth = 60;
    int photoHeight = 60;
    bool useOtherPhotoSources = false;

    if ( allowAsync() ) {

        Q_ASSERT( nodeHelper() );
        Q_ASSERT( sourceObject() );

        ContactDisplayMessageMemento *photoMemento =
                dynamic_cast<ContactDisplayMessageMemento*>( nodeHelper()->bodyPartMemento( message, "contactphoto" ) );
        if ( !photoMemento ) {
            const QString email = KPIMUtils::firstEmailAddress( message->from()->as7BitString(false) );
            photoMemento = new ContactDisplayMessageMemento( email );
            nodeHelper()->setBodyPartMemento( message, "contactphoto", photoMemento );
            QObject::connect( photoMemento, SIGNAL(update(MessageViewer::Viewer::UpdateMode)),
                              sourceObject(), SLOT(update(MessageViewer::Viewer::UpdateMode)) );

            QObject::connect( photoMemento, SIGNAL(changeDisplayMail(Viewer::ForceDisplayTo,bool)),
                              sourceObject(), SIGNAL(changeDisplayMail(Viewer::ForceDisplayTo,bool)) );
        }

        if ( photoMemento->finished() ) {

            useOtherPhotoSources = true;
            if ( photoMemento->photo().isIntern() )
            {
                // get photo data and convert to data: url
                QImage photo = photoMemento->photo().data();
                if ( !photo.isNull() )
                {
                    photoWidth = photo.width();
                    photoHeight = photo.height();
                    // scale below 60, otherwise it can get way too large
                    if ( photoHeight > 60 ) {
                        double ratio = ( double )photoHeight / ( double )photoWidth;
                        photoHeight = 60;
                        photoWidth = (int)( 60 / ratio );
                        photo = photo.scaled( photoWidth, photoHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
                    }
                    photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( photo );
                }
            }
            else
            {
                photoURL = photoMemento->photo().url();
                if ( photoURL.startsWith('/') )
                    photoURL.prepend( "file:" );
            }
        } else {
            // if the memento is not finished yet, use other photo sources instead
            useOtherPhotoSources = true;
        }
    } else {
        useOtherPhotoSources = true;
    }

    if( photoURL.isEmpty() && message->headerByType( "Face" ) && useOtherPhotoSources ) {
        // no photo, look for a Face header
        QString faceheader = message->headerByType( "Face" )->asUnicodeString();
        if ( !faceheader.isEmpty() ) {

            kDebug() << "Found Face: header";

            QByteArray facestring = faceheader.toUtf8();
            // Spec says header should be less than 998 bytes
            // Face: is 5 characters
            if ( facestring.length() < 993 ) {
               const QByteArray facearray = QByteArray::fromBase64( facestring );

                QImage faceimage;
                if ( faceimage.loadFromData( facearray, "png" ) ) {
                    // Spec says image must be 48x48 pixels
                    if ( ( 48 == faceimage.width() ) && ( 48 == faceimage.height() ) ) {
                        photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( faceimage );
                        photoWidth = 48;
                        photoHeight = 48;
                    } else {
                        kDebug() << "Face: header image is" << faceimage.width() << "by"
                                 << faceimage.height() << "not 48x48 Pixels";
                    }
                } else {
                    kDebug() << "Failed to load decoded png from Face: header";
                }
            } else {
                kDebug() << "Face: header too long at" << facestring.length();
            }
        }
    }

    if( photoURL.isEmpty() && message->headerByType( "X-Face" ) && useOtherPhotoSources )
    {
        // no photo, look for a X-Face header
        const QString xfhead = message->headerByType( "X-Face" )->asUnicodeString();
        if ( !xfhead.isEmpty() )
        {
            MessageViewer::KXFace xf;
            photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( xf.toImage( xfhead ) );
            photoWidth = 48;
            photoHeight = 48;

        }
    }

    QString userHTML;
    if( !photoURL.isEmpty() )
    {
        //kDebug() << "Got a photo:" << photoURL;
        userHTML = QString::fromLatin1("<img src=\"%1\" width=\"%2\" height=\"%3\">")
                .arg( photoURL ).arg( photoWidth ).arg( photoHeight );
        userHTML = QString("<div class=\"senderpic\">") + userHTML + "</div>";
    }

    // the subject line and box below for details
    if ( strategy->showHeader( "subject" ) ) {
        const int flags = LinkLocator::PreserveSpaces |
                ( GlobalSettings::self()->showEmoticons() ?
                      LinkLocator::ReplaceSmileys : 0 );

        headerStr += QString::fromLatin1("<div dir=\"%1\">%2</div>\n")
                .arg(subjectDir)
                .arg( MessageViewer::HeaderStyleUtil::subjectString( message, flags ) );
    }
    headerStr += "<table class=\"outer\"><tr><td width=\"100%\"><table>\n";
    //headerStr += "<table>\n";
    // from line
    // the mailto: URLs can contain %3 etc., therefore usage of multiple
    // QString::arg is not possible
    if ( strategy->showHeader( "from" ) ) {

        const QList<KMime::Types::Mailbox> resentFrom = MessageViewer::HeaderStyleUtil::resentFromList(message);
        headerStr += QString::fromLatin1("<tr><th>%1</th>\n"
                                         "<td>")
                .arg(i18n("From: "))
                + StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress )
                + ( message->headerByType( "Resent-From" ) ? "&nbsp;"
                                                             + i18n( "(resent from %1)",
                                                                     StringUtil::emailAddrAsAnchor(
                                                                         resentFrom, StringUtil::DisplayFullAddress ) )
                                                           : QString("") )
                + ( !vCardName().isEmpty() ? "&nbsp;&nbsp;<a href=\"" + vCardName() + "\">"
                                             + i18n("[vCard]") + "</a>"
                                           : QString("") )
                + ( !message->headerByType("Organization")
                    ? QString("")
                    : "&nbsp;&nbsp;("
                      + MessageViewer::HeaderStyleUtil::strToHtml(message->headerByType("Organization")->asUnicodeString())
                      + ')')
                + "</td></tr>\n";
    }
    // to line
    if ( strategy->showHeader( "to" ) )
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                             "<td>%2</td></tr>\n")
                         .arg( i18nc( "To-field of the mail header.","To: " ) )
                         .arg( StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress,
                                                              QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                              "FullToAddressList",
                                                              GlobalSettings::self()->numberOfAddressesToShow() ) ) );

    // cc line, if an
    if ( strategy->showHeader( "cc" ) && message->cc(false))
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                             "<td>%2</td></tr>\n")
                         .arg( i18n( "CC: " ) )
                         .arg( StringUtil::emailAddrAsAnchor(message->cc(), StringUtil::DisplayFullAddress,
                                                             QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                             "FullCcAddressList",
                                                             GlobalSettings::self()->numberOfAddressesToShow() ) ) );

    // Bcc line, if any
    if ( strategy->showHeader( "bcc" ) && message->bcc(false))
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                             "<td>%2</td></tr>\n")
                         .arg( i18n( "BCC: " ) )
                         .arg( StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ) ) );

    if ( strategy->showHeader( "date" ) )
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                             "<td dir=\"%2\">%3</td></tr>\n")
                         .arg(i18n("Date: "))
                         .arg( MessageViewer::HeaderStyleUtil::directionOf( MessageViewer::HeaderStyleUtil::dateStr( message->date()->dateTime() ) ) )
                         .arg(MessageViewer::HeaderStyleUtil::strToHtml( MessageViewer::HeaderStyleUtil::dateString( message, isPrinting(), /* short = */ false ) ) ) );
    if ( GlobalSettings::self()->showUserAgent() ) {
        if ( strategy->showHeader( "user-agent" ) ) {
            if ( message->headerByType("User-Agent") ) {
                headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                                     "<td>%2</td></tr>\n")
                                 .arg(i18n("User-Agent: "))
                                 .arg( MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("User-Agent")->as7BitString() ) ) );
            }
        }

        if ( strategy->showHeader( "x-mailer" ) ) {
            if ( message->headerByType("X-Mailer") ) {
                headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                                     "<td>%2</td></tr>\n")
                                 .arg(i18n("X-Mailer: "))
                                 .arg( MessageViewer::HeaderStyleUtil::strToHtml( message->headerByType("X-Mailer")->as7BitString() ) ) );
            }
        }
    }

    if ( strategy->showHeader( "x-bugzilla-url" ) && message->headerByType("X-Bugzilla-URL") ) {
        const QString product   = message->headerByType("X-Bugzilla-Product")   ? message->headerByType("X-Bugzilla-Product")->asUnicodeString() : QString();
        const QString component = message->headerByType("X-Bugzilla-Component") ? message->headerByType("X-Bugzilla-Component")->asUnicodeString() : QString();
        const QString status    = message->headerByType("X-Bugzilla-Status")    ? message->headerByType("X-Bugzilla-Status")->asUnicodeString() : QString();
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                             "<td>%2/%3, <strong>%4</strong></td></tr>\n")
                         .arg(i18n("Bugzilla: "))
                         .arg( MessageViewer::HeaderStyleUtil::strToHtml( product ) )
                         .arg( MessageViewer::HeaderStyleUtil::strToHtml( component ) )
                         .arg( MessageViewer::HeaderStyleUtil::strToHtml( status) ) );
    }

    headerStr.append( QString( "<tr><td colspan=\"2\"><div id=\"attachmentInjectionPoint\"></div></td></tr>" ) );
    headerStr.append(
                QString::fromLatin1( "</table></td><td align=\"center\">%1</td></tr></table>\n" ).arg(userHTML) );

    if ( !spamHTML.isEmpty() )
        headerStr.append( QString::fromLatin1( "<div class=\"spamheader\" dir=\"%1\"><b>%2</b>&nbsp;<span style=\"padding-left: 20px;\">%3</span></div>\n")
                          .arg( subjectDir, i18n("Spam Status:"), spamHTML ) );

    headerStr += "</div>\n\n";
    return headerStr;
}

}
