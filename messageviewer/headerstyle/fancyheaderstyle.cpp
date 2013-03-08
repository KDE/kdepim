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
#include "headerstyle/headerstyle_util.h"


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


QString FancyHeaderStyle::drawSpamMeter( SpamError spamError, double percent, double confidence,
                                         const QString & filterHeader, const QString & confidenceHeader )
{
    static const int meterWidth = 20;
    static const int meterHeight = 5;
    QImage meterBar( meterWidth, 1, QImage::Format_Indexed8/*QImage::Format_RGB32*/ );
    meterBar.setNumColors( 24 );

    meterBar.setColor( meterWidth + 1, qRgb( 255, 255, 255 ) );
    meterBar.setColor( meterWidth + 2, qRgb( 170, 170, 170 ) );
    if ( spamError != noError ) // grey is for errors
        meterBar.fill( meterWidth + 2 );
    else {
        static const unsigned short gradient[meterWidth][3] = {
            {   0, 255,   0 },
            {  27, 254,   0 },
            {  54, 252,   0 },
            {  80, 250,   0 },
            { 107, 249,   0 },
            { 135, 247,   0 },
            { 161, 246,   0 },
            { 187, 244,   0 },
            { 214, 242,   0 },
            { 241, 241,   0 },
            { 255, 228,   0 },
            { 255, 202,   0 },
            { 255, 177,   0 },
            { 255, 151,   0 },
            { 255, 126,   0 },
            { 255, 101,   0 },
            { 255,  76,   0 },
            { 255,  51,   0 },
            { 255,  25,   0 },
            { 255,   0,   0 }
        };

        meterBar.fill( meterWidth + 1 );
        const int max = qMin( meterWidth, static_cast<int>( percent ) / 5 );
        for ( int i = 0; i < max; ++i ) {
            meterBar.setColor( i+1, qRgb( gradient[i][0], gradient[i][1],
                    gradient[i][2] ) );
            meterBar.setPixel( i, 0, i+1 );
        }
    }

    QString titleText;
    QString confidenceString;
    if ( spamError == noError )
    {
        if ( confidence >= 0 )
        {
            confidenceString = QString::number( confidence ) + "% &nbsp;";
            titleText = i18n("%1% probability of being spam with confidence %3%.\n\n"
                             "Full report:\nProbability=%2\nConfidence=%4",
                             QString::number(percent,'f',2), filterHeader, confidence, confidenceHeader );
        }
        else // do not show negative confidence
        {
            confidenceString = QString() + "&nbsp;";
            titleText = i18n("%1% probability of being spam.\n\n"
                             "Full report:\nProbability=%2",
                             QString::number(percent,'f',2), filterHeader);
        }
    }
    else
    {
        QString errorMsg;
        switch ( spamError )
        {
        case errorExtractingAgentString:
            errorMsg = i18n( "No Spam agent" );
            break;
        case couldNotConverScoreToFloat:
            errorMsg = i18n( "Spam filter score not a number" );
            break;
        case couldNotConvertThresholdToFloatOrThresholdIsNegative:
            errorMsg = i18n( "Threshold not a valid number" );
            break;
        case couldNotFindTheScoreField:
            errorMsg = i18n( "Spam filter score could not be extracted from header" );
            break;
        case couldNotFindTheThresholdField:
            errorMsg = i18n( "Threshold could not be extracted from header" );
            break;
        default:
            errorMsg = i18n( "Error evaluating spam score" );
            break;
        }
        // report the error in the spam filter
        titleText = i18n("%1.\n\n"
                         "Full report:\n%2",
                         errorMsg, filterHeader );
    }
    return QString("<img src=\"%1\" width=\"%2\" height=\"%3\" style=\"border: 1px solid black;\" title=\"%4\"> &nbsp;")
            .arg( imgToDataUrl( meterBar ), QString::number( meterWidth ),
                  QString::number( meterHeight ), titleText ) + confidenceString;
}


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

    QString spamHTML;

    if ( GlobalSettings::self()->showSpamStatus() ) {
        const SpamScores scores = SpamHeaderAnalyzer::getSpamScores( message );

        for ( SpamScores::const_iterator it = scores.constBegin(), end = scores.constEnd() ; it != end ; ++it )
            spamHTML += (*it).agent() + ' ' +
                    drawSpamMeter( (*it).error(), (*it).score(), (*it).confidence(), (*it).spamHeader(), (*it).confidenceHeader() );
    }

    QString userHTML;

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
                    photoURL = imgToDataUrl( photo );
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
                QByteArray facearray = QByteArray::fromBase64( facestring );

                QImage faceimage;
                if ( faceimage.loadFromData( facearray, "png" ) ) {
                    // Spec says image must be 48x48 pixels
                    if ( ( 48 == faceimage.width() ) && ( 48 == faceimage.height() ) ) {
                        photoURL = imgToDataUrl( faceimage );
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
            photoURL = imgToDataUrl( xf.toImage( xfhead ) );
            photoWidth = 48;
            photoHeight = 48;

        }
    }

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

        // Get the resent-from header into a Mailbox
        QList<KMime::Types::Mailbox> resentFrom;
        if ( message->headerByType( "Resent-From" ) ) {
            const QByteArray data = message->headerByType( "Resent-From" )->as7BitString( false );
            const char * start = data.data();
            const char * end = start + data.length();
            KMime::Types::AddressList addressList;
            KMime::HeaderParsing::parseAddressList( start, end, addressList );
            foreach ( const KMime::Types::Address &addr, addressList ) {
                foreach ( const KMime::Types::Mailbox &mbox, addr.mailboxList ) {
                    resentFrom.append( mbox );
                }
            }
        }

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
                         .arg( MessageViewer::HeaderStyleUtil::directionOf( dateStr( message->date()->dateTime() ) ) )
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

QString FancyHeaderStyle::imgToDataUrl( const QImage &image )
{
    QByteArray ba;
    QBuffer buffer( &ba );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, "PNG" );
    return QString::fromLatin1("data:image/%1;base64,%2")
            .arg( QString::fromLatin1( "PNG" ), QString::fromLatin1( ba.toBase64() ) );
}

}
