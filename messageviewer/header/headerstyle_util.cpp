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

#include "headerstyle_util.h"
#include "viewer/nodehelper.h"
#include "header/headerstyle.h"

#include <messagecore/utils/stringutil.h>

#include "messagecore/settings/globalsettings.h"
#include "settings/globalsettings.h"



#include <KLocalizedString>
#include <KGlobal>

#include <QBuffer>
#include "contactdisplaymessagememento.h"
#include "kxface.h"

#include <kpimutils/email.h>

using namespace MessageCore;

namespace MessageViewer {
namespace HeaderStyleUtil {
//
// Convenience functions:
//
QString directionOf( const QString &str ) {
    return str.isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr");
}

QString strToHtml( const QString &str, int flags ) {
    return LinkLocator::convertToHtml( str, flags );
}

// Prepare the date string (when printing always use the localized date)
QString dateString( KMime::Message *message, bool printing, bool shortDate ) {
    const KDateTime dateTime = message->date()->dateTime();
    if ( !dateTime.isValid() )
        return i18nc( "Unknown date", "Unknown" );
    if( printing ) {
        KLocale * locale = KGlobal::locale();
        return locale->formatDateTime( dateTime );
    } else {
        if ( shortDate )
            return dateShortStr( dateTime );
        else
            return dateStr( dateTime );
    }
}

QString subjectString( KMime::Message *message, int flags )
{
    QString subjectStr;
    const KMime::Headers::Subject * const subject = message->subject(false);
    if ( subject ) {
        subjectStr = subject->asUnicodeString();
        if ( subjectStr.isEmpty() )
            subjectStr = i18n("No Subject");
        else
            subjectStr = strToHtml( subjectStr, flags );
    } else {
        subjectStr = i18n("No Subject");
    }
    return subjectStr;
}

QString subjectDirectionString( KMime::Message *message )
{
    QString subjectDir;
    if ( message->subject(false) )
        subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
    else
        subjectDir = directionOf( i18n("No Subject") );
    return subjectDir;
}

QString spamStatus(KMime::Message *message)
{
    QString spamHTML;
    if ( GlobalSettings::self()->showSpamStatus() ) {
        const SpamScores scores = SpamHeaderAnalyzer::getSpamScores( message );

        for ( SpamScores::const_iterator it = scores.constBegin(), end = scores.constEnd() ; it != end ; ++it )
            spamHTML += (*it).agent() + QLatin1Char(' ') +
                    MessageViewer::HeaderStyleUtil::drawSpamMeter( (*it).error(), (*it).score(), (*it).confidence(), (*it).spamHeader(), (*it).confidenceHeader() );
    }
    return spamHTML;
}


QString drawSpamMeter( SpamError spamError, double percent, double confidence,
                       const QString &filterHeader, const QString &confidenceHeader )
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
    if ( spamError == noError ) {
        if ( confidence >= 0 ) {
            confidenceString = QString::number( confidence ) + QLatin1String("% &nbsp;");
            titleText = i18n("%1% probability of being spam with confidence %3%.\n\n"
                             "Full report:\nProbability=%2\nConfidence=%4",
                             QString::number(percent,'f',2), filterHeader, confidence, confidenceHeader );
        } else { // do not show negative confidence
            confidenceString = QString() + QLatin1String("&nbsp;");
            titleText = i18n("%1% probability of being spam.\n\n"
                             "Full report:\nProbability=%2",
                             QString::number(percent,'f',2), filterHeader);
        }
    } else {
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
    return QString::fromLatin1("<img src=\"%1\" width=\"%2\" height=\"%3\" style=\"border: 1px solid black;\" title=\"%4\"> &nbsp;")
            .arg( imgToDataUrl( meterBar ), QString::number( meterWidth ),
                  QString::number( meterHeight ), titleText ) + confidenceString;
}

QString imgToDataUrl( const QImage &image )
{
    QByteArray ba;
    QBuffer buffer( &ba );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, "PNG" );
    return QString::fromLatin1("data:image/%1;base64,%2").arg( QString::fromLatin1( "PNG" ), QString::fromLatin1( ba.toBase64() ) );
}

QString dateStr(const KDateTime &dateTime)
{
    const time_t unixTime = dateTime.toTime_t();
    return KMime::DateFormatter::formatDate(
                static_cast<KMime::DateFormatter::FormatType>(
                    MessageCore::GlobalSettings::self()->dateFormat() ),
                unixTime, MessageCore::GlobalSettings::self()->customDateFormat() );
}

QString dateShortStr(const KDateTime &dateTime)
{
    return KGlobal::locale()->formatDateTime( dateTime, KLocale::FancyShortDate );
}


QList<KMime::Types::Mailbox> resentFromList(KMime::Message *message)
{
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
    return resentFrom;
}

QList<KMime::Types::Mailbox> resentToList(KMime::Message *message)
{
    // Get the resent-from header into a Mailbox
    QList<KMime::Types::Mailbox> resentTo;
    if ( message->headerByType( "Resent-To" ) ) {
        const QByteArray data = message->headerByType( "Resent-To" )->as7BitString( false );
        const char * start = data.data();
        const char * end = start + data.length();
        KMime::Types::AddressList addressList;
        KMime::HeaderParsing::parseAddressList( start, end, addressList );
        foreach ( const KMime::Types::Address &addr, addressList ) {
            foreach ( const KMime::Types::Mailbox &mbox, addr.mailboxList ) {
                resentTo.append( mbox );
            }
        }
    }
    return resentTo;
}

xfaceSettings xface(const MessageViewer::HeaderStyle *style, KMime::Message *message)
{

    xfaceSettings settings;
    bool useOtherPhotoSources = false;

    if ( style->allowAsync() ) {

        Q_ASSERT( style->nodeHelper() );
        Q_ASSERT( style->sourceObject() );

        ContactDisplayMessageMemento *photoMemento =
                dynamic_cast<ContactDisplayMessageMemento*>( style->nodeHelper()->bodyPartMemento( message, "contactphoto" ) );
        if ( !photoMemento ) {
            const QString email = QString::fromLatin1(KPIMUtils::firstEmailAddress( message->from()->as7BitString(false) ));
            photoMemento = new ContactDisplayMessageMemento( email );
            style->nodeHelper()->setBodyPartMemento( message, "contactphoto", photoMemento );
            QObject::connect( photoMemento, SIGNAL(update(MessageViewer::Viewer::UpdateMode)),
                              style->sourceObject(), SLOT(update(MessageViewer::Viewer::UpdateMode)) );

            QObject::connect( photoMemento, SIGNAL(changeDisplayMail(Viewer::DisplayFormatMessage,bool)),
                              style->sourceObject(), SIGNAL(changeDisplayMail(Viewer::DisplayFormatMessage,bool)) );
        }

        if ( photoMemento->finished() ) {

            useOtherPhotoSources = true;
            if ( photoMemento->photo().isIntern() ) {
                // get photo data and convert to data: url
                QImage photo = photoMemento->photo().data();
                if ( !photo.isNull() ) {
                    settings.photoWidth = photo.width();
                    settings.photoHeight = photo.height();
                    // scale below 60, otherwise it can get way too large
                    if ( settings.photoHeight > 60 ) {
                        double ratio = ( double )settings.photoHeight / ( double )settings.photoWidth;
                        settings.photoHeight = 60;
                        settings.photoWidth = (int)( 60 / ratio );
                        photo = photo.scaled( settings.photoWidth, settings.photoHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
                    }
                    settings.photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( photo );
                }
            } else {
                settings.photoURL = photoMemento->photo().url();
                if ( settings.photoURL.startsWith(QLatin1Char('/')) )
                    settings.photoURL.prepend( QLatin1String("file:") );
            }
        } else {
            // if the memento is not finished yet, use other photo sources instead
            useOtherPhotoSources = true;
        }
    } else {
        useOtherPhotoSources = true;
    }

    if( settings.photoURL.isEmpty() && message->headerByType( "Face" ) && useOtherPhotoSources ) {
        // no photo, look for a Face header
        const QString faceheader = message->headerByType( "Face" )->asUnicodeString();
        if ( !faceheader.isEmpty() ) {

            kDebug() << "Found Face: header";

            const QByteArray facestring = faceheader.toUtf8();
            // Spec says header should be less than 998 bytes
            // Face: is 5 characters
            if ( facestring.length() < 993 ) {
                const QByteArray facearray = QByteArray::fromBase64( facestring );

                QImage faceimage;
                if ( faceimage.loadFromData( facearray, "png" ) ) {
                    // Spec says image must be 48x48 pixels
                    if ( ( 48 == faceimage.width() ) && ( 48 == faceimage.height() ) ) {
                        settings.photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( faceimage );
                        settings.photoWidth = 48;
                        settings.photoHeight = 48;
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

    if( settings.photoURL.isEmpty() && message->headerByType( "X-Face" ) && useOtherPhotoSources ) {
        // no photo, look for a X-Face header
        const QString xfhead = message->headerByType( "X-Face" )->asUnicodeString();
        if ( !xfhead.isEmpty() ) {
            MessageViewer::KXFace xf;
            settings.photoURL = MessageViewer::HeaderStyleUtil::imgToDataUrl( xf.toImage( xfhead ) );
            settings.photoWidth = 48;
            settings.photoHeight = 48;
        }
    }

    return settings;
}

}
}
