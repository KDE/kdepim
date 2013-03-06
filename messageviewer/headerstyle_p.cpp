/*  -*- c++ -*-
    headerstyle.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

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

#include "headerstyle_p.h"
#include "headerstyle.h"

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
// Convenience functions:
//
static inline QString directionOf( const QString & str ) {
  return str.isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr");
}

static QString strToHtml( const QString & str,
                          int flags = LinkLocator::PreserveSpaces ) {
  return LinkLocator::convertToHtml( str, flags );
}

// Prepare the date string (when printing always use the localized date)
static QString dateString( KMime::Message *message, bool printing, bool shortDate ) {
  const KDateTime dateTime = message->date()->dateTime();
  if ( !dateTime.isValid() )
    return i18nc( "Unknown date", "Unknown" );
  if( printing ) {
    KLocale * locale = KGlobal::locale();
    return locale->formatDateTime( dateTime );
  } else {
    if ( shortDate )
      return MessageViewer::HeaderStyle::dateShortStr( dateTime );
    else
      return MessageViewer::HeaderStyle::dateStr( dateTime );
  }
}

static QString subjectString( KMime::Message *message, int flags = LinkLocator::PreserveSpaces )
{
  QString subject;
  if ( message->subject(false) ) {
    subject = message->subject()->asUnicodeString();
    if ( subject.isEmpty() )
      subject = i18n("No Subject");
    else
      subject = strToHtml( subject, flags );
  } else {
    subject = i18n("No Subject");
  }
  return subject;
}

static QString subjectDirectionString( KMime::Message *message )
{
  QString subjectDir;
  if ( message->subject(false) )
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );
  return subjectDir;
}


bool HeaderStyle::hasAttachmentQuickList() const
{
  return false;
}

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

  QString dir = QApplication::isRightToLeft() ? "rtl" : "ltr" ;

  // However, the direction of the message subject within the header is
  // determined according to the contents of the subject itself. Since
  // the "Re:" and "Fwd:" prefixes would always cause the subject to be
  // considered left-to-right, they are ignored when determining its
  // direction.

  const QString subjectDir = subjectDirectionString( message );

  QString headerStr = "<div class=\"header\" dir=\"" + dir + "\">\n";

  if ( strategy->showHeader( "subject" ) ) {
    headerStr += "<div dir=\"" + subjectDir + "\">\n"
                 "<b style=\"font-size:130%\">";

    headerStr += subjectString( message ) + "</b></div>\n";
  }
  QStringList headerParts;

  if ( strategy->showHeader( "from" ) ) {
/*TODO(Andras) review if it can happen or not
    if ( fromStr.isEmpty() ) // no valid email in from, maybe just a name
      fromStr = message->fromStrip(); // let's use that
*/
    QString fromPart = StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly );
    if ( !vCardName().isEmpty() )
      fromPart += "&nbsp;&nbsp;<a href=\"" + vCardName() + "\">" + i18n("[vCard]") + "</a>";
    headerParts << fromPart;
  }

  if ( strategy->showHeader( "cc" ) && message->cc(false) )
    headerParts << i18n("CC: ") + StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayNameOnly );

  if ( strategy->showHeader( "bcc" ) && message->bcc(false) )
    headerParts << i18n("BCC: ") + StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayNameOnly );

  if ( strategy->showHeader( "date" ) )
    headerParts << strToHtml( dateString( message, isPrinting(), /* shortDate = */ true ) );

  // remove all empty (modulo whitespace) entries and joins them via ", \n"
  headerStr += " (" + headerParts.filter( QRegExp( "\\S" ) ).join( ",\n" ) + ')';

  headerStr += "</div>\n";

  // ### iterate over the rest of strategy->headerToDisplay() (or
  // ### all headers if DefaultPolicy == Display) (elsewhere, too)
  return headerStr;
}

//
// PlainHeaderStyle:
//   show every header field on a line by itself,
//   show subject larger
//


QString PlainHeaderStyle::format( KMime::Message *message ) const {
  if ( !message ) return QString();
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

  const QString subjectDir = subjectDirectionString( message );
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
                         subjectString( message ) + "</b></div>\n")
                      .arg(subjectDir);

  if ( strategy->showHeader( "date" ) )
    headerStr.append(i18n("Date: ") + strToHtml( dateString(message, isPrinting(), /* short = */ false ) ) + "<br/>\n" );

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
            strToHtml(message->headerByType("Organization")->asUnicodeString()) + ')');
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
    result += strToHtml( QLatin1String(header->type()) + QLatin1String(": ") + header->asUnicodeString() );
    result += QLatin1String( "<br />\n" );
    delete header;
    header = KMime::HeaderParsing::extractFirstHeader( head );
  }

  return result;
}

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
  if ( !message ) return QString();
  const HeaderStrategy *strategy = headerStrategy();
  if ( !strategy )
    strategy = HeaderStrategy::rich();

  // ### from kmreaderwin begin
  // The direction of the header is determined according to the direction
  // of the application layout.

  QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
  QString headerStr = QString::fromLatin1("<div class=\"fancy header\" dir=\"%1\">\n").arg(dir);

  // However, the direction of the message subject within the header is
  // determined according to the contents of the subject itself. Since
  // the "Re:" and "Fwd:" prefixes would always cause the subject to be
  // considered left-to-right, they are ignored when determining its
  // direction.

  const QString subjectDir = subjectDirectionString( message );

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
                      .arg( subjectString( message, flags ) );
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
                              + strToHtml(message->headerByType("Organization")->asUnicodeString())
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
                  .arg( directionOf( dateStr( message->date()->dateTime() ) ) )
                          .arg(strToHtml( dateString( message, isPrinting(), /* short = */ false ) ) ) );
  if ( GlobalSettings::self()->showUserAgent() ) {
    if ( strategy->showHeader( "user-agent" ) ) {
      if ( message->headerByType("User-Agent") ) {
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                  "<td>%2</td></tr>\n")
                          .arg(i18n("User-Agent: "))
                          .arg( strToHtml( message->headerByType("User-Agent")->as7BitString() ) ) );
      }
    }

    if ( strategy->showHeader( "x-mailer" ) ) {
      if ( message->headerByType("X-Mailer") ) {
        headerStr.append(QString::fromLatin1("<tr><th>%1</th>\n"
                                  "<td>%2</td></tr>\n")
                          .arg(i18n("X-Mailer: "))
                          .arg( strToHtml( message->headerByType("X-Mailer")->as7BitString() ) ) );
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
                      .arg( strToHtml( product ) )
                      .arg( strToHtml( component ) )
                      .arg( strToHtml( status) ) );
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

// #####################

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

  const QString subjectDir = subjectDirectionString( message );

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
    + dateString( message, isPrinting(), /* shortDate */ false ) + "</span></div>"
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
    headerStr += subjectString( message )+ "</td> \n"
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

// #####################


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
  QString linkColor ="style=\"color: #0E49A1; text-decoration: none\"";
  QString fromPart = StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, linkColor );

  if ( !style->vCardName().isEmpty() )
    fromPart += "&nbsp;&nbsp;<a href=\"" + style->vCardName() + "\" " + linkColor + ">" + i18n( "[vCard]" ) + "</a>";

  const QString toPart = StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor );
  const QString ccPart = StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor );

  // Background image
  const QString imagePath( QLatin1String( "file:///" ) + KStandardDirs::locate( "data", "libmessageviewer/pics/" ) );
  const QString mobileImagePath( imagePath + QLatin1String( "mobile_" ) );
  const QString mobileExtendedImagePath( imagePath + QLatin1String( "mobileextended_" ) );

  const Akonadi::MessageStatus status = style->messageStatus();
  QString flagsPart;
  if ( status.isImportant() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_important.png\" height=\"22\" width=\"22\"/>";
  if ( status.hasAttachment() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_attachment.png\" height=\"22\" width=\"22\"/>";
  if ( status.isToAct() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_actionitem.png\" height=\"22\" width=\"22\"/>";
  if ( status.isReplied() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_replied.png\" height=\"22\" width=\"22\"/>";
  if ( status.isForwarded() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_forwarded.png\" height=\"22\" width=\"22\"/>";
  if ( status.isSigned() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_signed.png\" height=\"22\" width=\"22\"/>";
  if ( status.isEncrypted() )
    flagsPart += "<img src=\"" + mobileImagePath + "status_encrypted.png\" height=\"22\" width=\"22\"/>";


  QString headerStr;
  headerStr += "<div style=\"width: 100%\">\n";
  headerStr += "  <table width=\"100%\" bgcolor=\"#B4E3F7\">\n";
  headerStr += "    <tr>\n";
  headerStr += "      <td valign=\"bottom\" width=\"80%\">\n";
  headerStr += "        <div style=\"text-align: left; font-size: 20px; color: #0E49A1\">" + fromPart + "</div>\n";
  headerStr += "      </td>\n";
  headerStr += "      <td valign=\"bottom\" width=\"20%\" align=\"right\">\n";
  headerStr += "        <div style=\"text-align: right; color: #0E49A1;\">" + flagsPart + "</div>\n";
  headerStr += "      </td>\n";
  headerStr += "    </tr>\n";
  headerStr += "  </table>\n";
  headerStr += "  <table width=\"100%\" bgcolor=\"#B4E3F7\">\n";
  if ( extendedFormat ) {
    headerStr += "    <tr>\n";
    headerStr += "      <td valign=\"bottom\" colspan=\"2\">\n";
    headerStr += "        <div style=\"height: 20px; font-size: 15px; color: #0E49A1\">" + toPart + ccPart + "</div>\n";
    headerStr += "      </td>\n";
    headerStr += "    </tr>\n";
  }

  const int subjectFontSize = matchingFontSize( message->subject()->asUnicodeString(), 650, 20 );
  headerStr += "    <tr>\n";
  headerStr += "      <td valign=\"bottom\" colspan=\"2\">\n";
  headerStr += "        <div style=\"height: 35px; font-size: " + QString::number( subjectFontSize ) + "px; color: #24353F;\">" + message->subject()->asUnicodeString() + "</div>\n";
  headerStr += "      </td>\n";
  headerStr += "    </tr>\n";
  headerStr += "    <tr>\n";
  headerStr += "      <td align=\"left\" width=\"50%\">\n";
  if ( !style->messagePath().isEmpty() ) {
    headerStr += "        <div style=\"font-size: 15px; color: #24353F\">" + style->messagePath() + "</div>\n";
  }
  headerStr += "      </td>\n";
  headerStr += "      <td align=\"right\" width=\"50%\">\n";
  headerStr += "        <div style=\"font-size: 15px; color: #24353F; text-align: right; margin-right: 15px\">" + i18n( "sent: " );
  headerStr += dateString( message, style->isPrinting(), /* shortDate = */ false ) + "</div>\n";
  headerStr += "      </td>\n";
  headerStr += "    </tr>\n";
  headerStr += "  </table>\n";
  headerStr += "  <br/>\n";
  headerStr += "</div>\n";

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


//
// CustomHeaderStyle:
//   show every header field on a line by itself,
//   show subject larger
//


QString CustomHeaderStyle::format( KMime::Message *message ) const {
  if ( !message ) return QString();
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

  const QString subjectDir = subjectDirectionString( message );
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
                         subjectString( message ) + "</b></div>\n")
                      .arg(subjectDir);

  if ( strategy->showHeader( "date" ) )
    headerStr.append(i18n("Date: ") + strToHtml( dateString(message, isPrinting(), /* short = */ false ) ) + "<br/>\n" );

  if ( strategy->showHeader( "from" ) ) {
    headerStr.append( i18n("From: ") +
                      StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, "", StringUtil::ShowLink ) );
    if ( !vCardName().isEmpty() )
      headerStr.append("&nbsp;&nbsp;<a href=\"" + vCardName() +
            "\">" + i18n("[vCard]") + "</a>" );

    if ( strategy->showHeader( "organization" )
        && message->headerByType("Organization"))
      headerStr.append("&nbsp;&nbsp;(" +
            strToHtml(message->headerByType("Organization")->asUnicodeString()) + ')');
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

QString CustomHeaderStyle::formatAllMessageHeaders( KMime::Message *message ) const {
  QByteArray head = message->head();
  KMime::Headers::Base *header = KMime::HeaderParsing::extractFirstHeader( head );
  QString result;
  while ( header ) {
    result += strToHtml( QLatin1String(header->type()) + QLatin1String(": ") + header->asUnicodeString() );
    result += QLatin1String( "<br />\n" );
    delete header;
    header = KMime::HeaderParsing::extractFirstHeader( head );
  }
  return result;
}

}
