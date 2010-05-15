/*  -*- c++ -*-
    headerstyle.cpp

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

#include "headerstyle.h"

#include "headerstrategy.h"
#include <kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;
#include "spamheaderanalyzer.h"
#include "globalsettings.h"
#include "nodehelper.h"
#include "contactphotomemento.h"

#include <kpimutils/email.h>
#include "kxface.h"
#include <messagecore/stringutil.h>
#include "messagecore/globalsettings.h"

#include <akonadi/contact/contactsearchjob.h>
#include <kdebug.h>
#include <kconfiggroup.h>
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

#include <kstandarddirs.h>
#include <KApplication>

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

using namespace MessageCore;

//
// Convenience functions:
//
static inline QString directionOf( const QString & str ) {
  return str.isRightToLeft() ? "rtl" : "ltr" ;
}

// ### tmp wrapper to make kmreaderwin code working:
static QString strToHtml( const QString & str,
                          int flags = LinkLocator::PreserveSpaces ) {
  return LinkLocator::convertToHtml( str, flags );
}

// Prepare the date string (when printing always use the localized date)
static QString dateString( KMime::Message *message, bool printing, bool shortDate ) {
  if( printing ) {
    KDateTime dateTime = message->date()->dateTime();
    KLocale * locale = KGlobal::locale();
    return locale->formatDateTime( dateTime );
  } else {
    if ( shortDate )
      return MessageViewer::HeaderStyle::dateStr( message->date()->dateTime() );
    else
      return MessageViewer::HeaderStyle::dateShortStr( message->date()->dateTime() );
  }
}

//
// BriefHeaderStyle
//   Show everything in a single line, don't show header field names.
//
namespace MessageViewer {
class BriefHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  BriefHeaderStyle() : HeaderStyle() {}
  virtual ~BriefHeaderStyle() {}

public:
  const char * name() const { return "brief"; }
  HeaderStyle * next() const { return plain(); }
  HeaderStyle * prev() const { return fancy(); }

  QString format( KMime::Message *message ) const;
};

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

  QString subjectDir;
  if ( message->subject(false) )
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );

  QString headerStr = "<div class=\"header\" dir=\"" + dir + "\">\n";

  if ( strategy->showHeader( "subject" ) )
    headerStr += "<div dir=\"" + subjectDir + "\">\n"
                  "<b style=\"font-size:130%\">" +
                  strToHtml( message->subject()->asUnicodeString() ) +
                  "</b></div>\n";

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

class PlainHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  PlainHeaderStyle() : HeaderStyle() {}
  virtual ~PlainHeaderStyle() {}

public:
  const char * name() const { return "plain"; }
  HeaderStyle * next() const { return fancy(); }
  HeaderStyle * prev() const { return brief(); }

  QString format( KMime::Message *message ) const;

private:
  QString formatAllMessageHeaders( KMime::Message *message ) const;
};

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

  QString subjectDir;
  if (message->subject(false))
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );

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
                          strToHtml(message->subject()->asUnicodeString()) + "</b></div>\n")
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
  KMime::Headers::Generic *header = message->nextHeader(head);
  QString result;
  while ( header ) {
    result += QLatin1String(header->type()) + ": ";
    result += strToHtml( header->asUnicodeString() );
    delete header;
    header = message->nextHeader(head);
  }

  return result;
}

//
// FancyHeaderStyle:
//   Like PlainHeaderStyle, but with slick frames and background colours.
//

class FancyHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  FancyHeaderStyle() : HeaderStyle() {}
  virtual ~FancyHeaderStyle() {}

public:
  const char * name() const { return "fancy"; }
  HeaderStyle * next() const { return enterprise(); }
  HeaderStyle * prev() const { return plain(); }

  QString format( KMime::Message *message ) const;
  static QString imgToDataUrl( const QImage & image );

private:
  static QString drawSpamMeter( SpamError spamError, double percent, double confidence,
  const QString & filterHeader, const QString & confidenceHeader );
};

QString FancyHeaderStyle::drawSpamMeter( SpamError spamError, double percent, double confidence,
                                          const QString & filterHeader, const QString & confidenceHeader )
{
  static const int meterWidth = 20;
  static const int meterHeight = 5;
  QImage meterBar( meterWidth, 1, QImage::Format_Indexed8/*QImage::Format_RGB32*/ );
  meterBar.setNumColors( 24 );

  const unsigned short gradient[meterWidth][3] = {
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
  meterBar.setColor( meterWidth + 1, qRgb( 255, 255, 255 ) );
  meterBar.setColor( meterWidth + 2, qRgb( 170, 170, 170 ) );
  if ( spamError != noError ) // grey is for errors
    meterBar.fill( meterWidth + 2 );
  else {
    meterBar.fill( meterWidth + 1 );
    int max = qMin( meterWidth, static_cast<int>( percent ) / 5 );
    for ( int i = 0; i < max; ++i ) {
      meterBar.setColor( i+1, qRgb( gradient[i][0], gradient[i][1],
                                    gradient[i][2] ) );
      meterBar.setPixel( i, 0, i+1 );
    }
  }

  QString titleText;
  QString confidenceString = QString();
  if ( spamError == noError )
  {
    if ( confidence >= 0 )
    {
      confidenceString = QString::number( confidence ) + "% &nbsp;";
      titleText = i18n("%1% probability of being spam with confidence %3%.\n\n"
                       "Full report:\nProbability=%2\nConfidence=%4",
                       percent, filterHeader, confidence, confidenceHeader );
    }
    else // do not show negative confidence
    {
      confidenceString = QString() + "&nbsp;";
      titleText = i18n("%1% probability of being spam.\n\n"
                       "Full report:\nProbability=%2",
                       percent, filterHeader);
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
  QString headerStr = QString("<div class=\"fancy header\" dir=\"%1\">\n").arg(dir);

  // However, the direction of the message subject within the header is
  // determined according to the contents of the subject itself. Since
  // the "Re:" and "Fwd:" prefixes would always cause the subject to be
  // considered left-to-right, they are ignored when determining its
  // direction.

  QString subjectDir;
  if ( message->subject(false) )
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );

  // Spam header display.
  // If the spamSpamStatus config value is true then we look for headers
  // from a few spam filters and try to create visually meaningful graphics
  // out of the spam scores.

  QString spamHTML;

  if ( GlobalSettings::self()->showSpamStatus() ) {
    SpamScores scores = SpamHeaderAnalyzer::getSpamScores( message );
    for ( SpamScoresIterator it = scores.begin(); it != scores.end(); ++it )
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

    ContactPhotoMemento *photoMemento =
        dynamic_cast<ContactPhotoMemento*>( nodeHelper()->bodyPartMemento( message, "contactphoto" ) );
    if ( !photoMemento ) {
      const QString email = KPIMUtils::firstEmailAddress( message->from()->asUnicodeString() );
      photoMemento = new ContactPhotoMemento( email );
      nodeHelper()->setBodyPartMemento( message, "contactphoto", photoMemento );
      QObject::connect( photoMemento, SIGNAL( update( Viewer::UpdateMode ) ),
                        sourceObject(), SLOT( update( Viewer::UpdateMode ) ) );
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
    QString xfhead = message->headerByType( "X-Face" )->asUnicodeString();
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
    userHTML = QString("<img src=\"%1\" width=\"%2\" height=\"%3\">")
        .arg( photoURL ).arg( photoWidth ).arg( photoHeight );
    userHTML = QString("<div class=\"senderpic\">") + userHTML + "</div>";
  }

  // the subject line and box below for details
  if ( strategy->showHeader( "subject" ) ) {
    const int flags = LinkLocator::PreserveSpaces |
                ( GlobalSettings::self()->showEmoticons() ?
                  LinkLocator::ReplaceSmileys : 0 );

    headerStr += QString("<div dir=\"%1\">%2</div>\n")
                      .arg(subjectDir)
                      .arg(!message->subject(false)?
                            i18n("No Subject") :
                            strToHtml( message->subject()->asUnicodeString(), flags ));
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

    headerStr += QString("<tr><th>%1</th>\n"
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
    headerStr.append(QString("<tr><th>%1</th>\n"
                  "<td>%2</td></tr>\n")
                          .arg( i18nc( "To-field of the mail header.","To: " ) )
                          .arg( StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress,
                                                             QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                             "FullToAddressList",
                                                             GlobalSettings::self()->numberOfAddressesToShow() ) ) );

  // cc line, if an
  if ( strategy->showHeader( "cc" ) && message->cc(false))
    headerStr.append(QString("<tr><th>%1</th>\n"
                  "<td>%2</td></tr>\n")
                            .arg( i18n( "CC: " ) )
                            .arg( StringUtil::emailAddrAsAnchor(message->cc(), StringUtil::DisplayFullAddress,
                                                               QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                               "FullCcAddressList",
                                                               GlobalSettings::self()->numberOfAddressesToShow() ) ) );

  // Bcc line, if any
  if ( strategy->showHeader( "bcc" ) && message->bcc(false))
    headerStr.append(QString("<tr><th>%1</th>\n"
                  "<td>%2</td></tr>\n")
                            .arg( i18n( "BCC: " ) )
                            .arg( StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ) ) );

  if ( strategy->showHeader( "date" ) )
    headerStr.append(QString("<tr><th>%1</th>\n"
                  "<td dir=\"%2\">%3</td></tr>\n")
                          .arg(i18n("Date: "))
                  .arg( directionOf( dateStr( message->date()->dateTime() ) ) )
                          .arg(strToHtml( dateString( message, isPrinting(), /* short = */ false ) ) ) );
  if ( GlobalSettings::self()->showUserAgent() ) {
    if ( strategy->showHeader( "user-agent" ) ) {
      if ( message->headerByType("User-Agent") ) {
        headerStr.append(QString("<tr><th>%1</th>\n"
                                  "<td>%2</td></tr>\n")
                          .arg(i18n("User-Agent: "))
                          .arg( strToHtml( message->headerByType("User-Agent")->as7BitString() ) ) );
      }
    }

    if ( strategy->showHeader( "x-mailer" ) ) {
      if ( message->headerByType("X-Mailer") ) {
        headerStr.append(QString("<tr><th>%1</th>\n"
                                  "<td>%2</td></tr>\n")
                          .arg(i18n("X-Mailer: "))
                          .arg( strToHtml( message->headerByType("X-Mailer")->as7BitString() ) ) );
      }
    }
  }
  headerStr.append( QString( "<tr><td colspan=\"2\"><div id=\"attachmentInjectionPoint\"></div></td></tr>" ) );
  headerStr.append(
        QString( "</table></td><td align=\"center\">%1</td></tr></table>\n" ).arg(userHTML) );

  if ( !spamHTML.isEmpty() )
    headerStr.append( QString( "<div class=\"spamheader\" dir=\"%1\"><b>%2</b>&nbsp;<span style=\"padding-left: 20px;\">%3</span></div>\n")
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

class EnterpriseHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  EnterpriseHeaderStyle() : HeaderStyle() {}
  virtual ~EnterpriseHeaderStyle() {}

public:
  const char * name() const { return "enterprise"; }
  HeaderStyle * next() const {
#if defined KDEPIM_MOBILE_UI
    return mobile();
#else
    return brief();
#endif
  }
  HeaderStyle * prev() const { return fancy(); }

  QString format( KMime::Message *message ) const;
};

QString EnterpriseHeaderStyle::format( KMime::Message *message ) const
{
  if ( !message ) return QString();
  const HeaderStrategy *strategy = headerStrategy();
  if ( !strategy ) {
    strategy = HeaderStrategy::brief();
  }

  // The direction of the header is determined according to the direction
  // of the application layout.

  QString dir = ( QApplication::layoutDirection() == Qt::RightToLeft ) ?
      "rtl" : "ltr" ;

  // However, the direction of the message subject within the header is
  // determined according to the contents of the subject itself. Since
  // the "Re:" and "Fwd:" prefixes would always cause the subject to be
  // considered left-to-right, they are ignored when determining its
  // direction.

//TODO(Andras) this is duplicate code, try to factor out!
  QString subjectDir;
  if (message->subject(false))
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );

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
  imgpath.prepend( "file://" );
  imgpath.append("enterprise_");
  const QString borderSettings( " padding-top: 0px; padding-bottom: 0px; border-width: 0px " );
  QString headerStr;

  // 3D borders
  if(isTopLevel())
    headerStr +=
      "<div style=\"position: fixed; top: 0px; left: 0px; background-color: #606060; "
      "background-image: url("+imgpath+"s_left.png); width: 10px; min-height: 100%;\">&nbsp;</div>"
      "<div style=\"position: fixed; top: 0px; right: 0px;  background-color: #606060; "
      "background-image: url("+imgpath+"s_right.png); width: 10px; min-height: 100%;\">&nbsp;</div>";

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
    "   <td style=\"\"> \n"
    "    <table style=\"color: "+fontColor.name()+" ! important; margin: 1px; border-spacing: 0px;\" cellpadding=0> \n";

  // subject
  //strToHtml( message->subject() )
  if ( strategy->showHeader( "subject" ) ) {
    headerStr +=
      "     <tr> \n"
      "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; "+borderSettings+"\"></td> \n"
      "      <td style=\"font-weight: bolder; font-size: 120%; padding-right: 91px; "+borderSettings+"\">"+message->subject()->asUnicodeString()+"</td> \n"
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
      "      <td style=\"font-size: 10px; padding-left: 5px; padding-right: 24px; text-align: right; "+borderSettings+"\">"+i18n("From: ")+"</td> \n"
      "      <td style=\""+borderSettings+"\">"+ fromPart +"</td> "
      "     </tr> ";
  }

  // to line
  if ( strategy->showHeader( "to" ) ) {
    headerStr +=
      "     <tr> "
      "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("To: ") + "</td> "
      "      <td style=\"" + borderSettings + "\">" +
      StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor ) +
      "      </td> "
      "     </tr>\n";
  }

  // cc line, if any
  if ( strategy->showHeader( "cc" ) && message->cc( false ) ) {
    headerStr +=
      "     <tr> "
      "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("CC: ") + "</td> "
      "      <td style=\"" + borderSettings + "\">" +
      StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor ) +
      "      </td> "
      "     </tr>\n";
  }

  // bcc line, if any
  if ( strategy->showHeader( "bcc" ) && message->bcc( false ) ) {
    headerStr +=
      "     <tr> "
      "      <td style=\"font-size: 6px; text-align: right; padding-left: 5px; padding-right: 24px; " + borderSettings + "\">" + i18n("BCC: ") + "</td> "
      "      <td style=\"" + borderSettings + "\">" +
      StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress, linkColor ) +
      "      </td> "
      "     </tr>\n";
  }

  // header-bottom
  headerStr +=
    "    </table> \n"
    "   </td> \n"
    "   <td style=\"min-width: 6px; max-height: 15px; background: url("+imgpath+"right.png); \"></td> \n"
    "  </tr> \n"
    "  <tr> \n"
    "   <td style=\"min-width: 6px; background: url("+imgpath+"s_left.png); \"></td> \n"
    "   <td style=\"height: 35px; width: 80%; background: url("+imgpath+"sbar.png);\"> \n"
    "    <img src=\""+imgpath+"sw.png\" style=\"margin: 0px; height: 30px; overflow:hidden; \"> \n"
    "    <img src=\""+imgpath+"sp_right.png\" style=\"float: right; \"> </td> \n"
    "   <td style=\"min-width: 6px; background: url("+imgpath+"s_right.png); \"></td> \n"
    "  </tr> \n"
    " </table> \n";

  // kmail icon
  if( isTopLevel() ) {
    headerStr +=
      "<div class=\"noprint\" style=\"position: absolute; top: -14px; right: 30px; width: 91px; height: 91px;\">\n"
      "<img style=\"float: right;\" src=\""+imgpath+"icon.png\">\n"
      "</div>\n";

    // attachments
    headerStr +=
      "<div class=\"noprint\" style=\"position: absolute; top: 60px; right: 20px;\">"
      "<div id=\"attachmentInjectionPoint\"></div>"
      "</div>\n";
  }

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

#ifdef KDEPIM_MOBILE_UI

class MobileHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  MobileHeaderStyle() : HeaderStyle() {}
  virtual ~MobileHeaderStyle() {}

public:
  const char * name() const { return "mobile"; }
  HeaderStyle * next() const { return brief(); }
  HeaderStyle * prev() const { return enterprise(); }

  QString format( KMime::Message *message ) const;
};

QString MobileHeaderStyle::format( KMime::Message *message ) const
{
  if ( !message ) return QString();

  // Bertjan: This is a highly simplified header for mobile devices. It lacks
  //          currently about almost everything except for the stuff that was in
  //          the mockup of Nuno. We might want to add additional items such as
  //          encryption I guess.

  // Use always the brief strategy for the mobile headers.
  const HeaderStrategy *strategy = HeaderStrategy::brief();

  // From
  QString linkColor ="style=\"color: #0E49A1; text-decoration: none\"";
  QString fromPart = StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly, linkColor );

  if ( !vCardName().isEmpty() )
    fromPart += "&nbsp;&nbsp;<a href=\"" + vCardName() + "\" " + linkColor + ">" + i18n( "[vCard]" ) + "</a>";

  // Background image
  QString imgpath( KStandardDirs::locate("data","libmessageviewer/pics/") );
  imgpath.prepend( "file://" );
  imgpath.append("mobile_");

  QString headerStr;
  headerStr += "<div style=\"position: absolute;\n";
  headerStr += "            top: 5px;\n";
  headerStr += "            left: 0px;\n";
  headerStr += "            width: 100%;\n";
  headerStr += "            height: 94px;\n";
  headerStr += "            background-image: url('" + imgpath + "bg.png');\n";
  headerStr += "            background-repeat: repeat-x;\">\n";
  headerStr += "<table style=\"margin-left: 0px; margin-top:2px; width: 100%\">\n";

  // From line
  headerStr += "<tr style=\"height: 30px; vertical-align: middle; font-size: 20px; color: #0E49A1;\">\n";
  headerStr += "  <td style=\"width: 120px; text-align: right; margin-right: 7px;\">" + i18n( "From:" ) + "</td>\n";
  headerStr += "  <td colspan=\"2\">" + fromPart + "</td>\n";
  headerStr += "</tr>\n";

  // Subject line
  headerStr += "<tr style=\"height: 30px; font-size: 20px; color: #24353F;\">\n";
  headerStr += "  <td style=\"text-align: right; margin-right: 7px;\">" + i18n( "Subject:" ) + "</td>\n";
  headerStr += "  <td colspan=\"2\" style=\"white-space: nowrap\">" + message->subject()->asUnicodeString() + "</td>\n";
  headerStr += "</tr>\n";

  headerStr += "<tr style=\"margin-top: 2px; height: 27px; font-size: 15px; color: #24353F;\">\n";

  if ( !messagePath().isEmpty() )
  {
    // TODO: Put these back in when we can somehow determine the path
    //headerStr += "  <td style=\"text-align: right; margin-right: 7px;\">" + i18n( "in:" )+ "</td>\n";
    headerStr += "  <td colspan=\"2\" style=\"padding-left: 50px;\">" + messagePath() + "</td>\n";
  }
  headerStr += "  <td>&nbsp;</td>\n";
  headerStr += "  <td colspan=\"2\" style=\"text-align: right; margin-right: 15px;\">sent: ";
  headerStr += dateString( message, isPrinting(), /* shortDate = */ false ) + "</td>\n";
  headerStr += "</tr>\n";
  headerStr += "</table>\n";
  headerStr += "</div>\n";
  headerStr += "<div style=\"margin-left: 40px; position: absolute; top: 110px;\">\n";

  //kDebug() << headerStr;
  return headerStr;
}

#endif

// #####################

//
// HeaderStyle abstract base:
//

HeaderStyle::HeaderStyle()
  : mStrategy( 0 ), mPrinting( false ), mTopLevel( true ), mNodeHelper( 0 ), mAllowAsync( false ),
    mSourceObject( 0 )
{
}

HeaderStyle::~HeaderStyle() {

}

HeaderStyle * HeaderStyle::create( Type type ) {
  switch ( type ) {
  case Brief:  return brief();
  case Plain:  return plain();
  case Fancy:   return fancy();
  case Enterprise: return enterprise();
#ifdef KDEPIM_MOBILE_UI
  case Mobile: return mobile();
#endif
  }
  kFatal() << "Unknown header style ( type ==" << (int)type << ") requested!";
  return 0; // make compiler happy
}

HeaderStyle * HeaderStyle::create( const QString & type ) {
  const QString lowerType = type.toLower();
  if ( lowerType == "brief" ) return brief();
  if ( lowerType == "plain" )  return plain();
  if ( lowerType == "enterprise" )  return enterprise();
#ifdef KDEPIM_MOBILE_UI
  if ( lowerType ==  "mobile" )  return mobile();
#endif
  //if ( lowerType == "fancy" ) return fancy(); // not needed, see below
  // don't kFatal here, b/c the strings are user-provided
  // (KConfig), so fail gracefully to the default:
  return fancy();
}

HeaderStyle * briefStyle = 0;
HeaderStyle * plainStyle = 0;
HeaderStyle * fancyStyle = 0;
HeaderStyle * enterpriseStyle = 0;
#ifdef KDEPIM_MOBILE_UI
HeaderStyle * mobileStyle = 0;
#endif

HeaderStyle * HeaderStyle::brief() {
  if ( !briefStyle )
    briefStyle = new BriefHeaderStyle();
  return briefStyle;
}

HeaderStyle * HeaderStyle::plain() {
  if ( !plainStyle )
    plainStyle = new PlainHeaderStyle();
  return plainStyle;
}

HeaderStyle * HeaderStyle::fancy() {
  if ( !fancyStyle )
    fancyStyle = new FancyHeaderStyle();
  return fancyStyle;
}

HeaderStyle * HeaderStyle::enterprise() {
  if ( !enterpriseStyle )
    enterpriseStyle = new EnterpriseHeaderStyle();
  return enterpriseStyle;
}

#ifdef KDEPIM_MOBILE_UI
HeaderStyle * HeaderStyle::mobile() {
  if ( !mobileStyle )
    mobileStyle = new MobileHeaderStyle();
  return mobileStyle;
}
#endif

QString HeaderStyle::dateStr(const KDateTime &dateTime)
{
  const time_t unixTime = dateTime.toTime_t();
  return KMime::DateFormatter::formatDate(
              static_cast<KMime::DateFormatter::FormatType>(
                  MessageCore::GlobalSettings::self()->dateFormat() ),
              unixTime, MessageCore::GlobalSettings::self()->customDateFormat() );
}

QByteArray HeaderStyle::dateShortStr(const KDateTime &dateTime)
{
    const time_t unixTime = dateTime.toTime_t();

    QByteArray result = ctime(&unixTime);

    if (result[result.length()-1]=='\n')
        result.truncate(result.length()-1);

    return result;
}

}
