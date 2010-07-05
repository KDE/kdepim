/*  -*- c++ -*-
    headertheme.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2010 Ronny Yabar Aizcorbe

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

#include "headertheme.h"

#include <kpimutils/linklocator.h>
using KPIMUtils::LinkLocator;
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

// Grantlee
#include <grantlee/template.h>
#include <grantlee/context.h>
#include <grantlee/engine.h>

#include "grantlee_paths.h"

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
      return MessageViewer::HeaderTheme::dateStr( message->date()->dateTime() );
    else
      return MessageViewer::HeaderTheme::dateShortStr( message->date()->dateTime() );
  }
}

namespace MessageViewer {

HeaderTheme::HeaderTheme()
  : mPrinting( false ), mTopLevel( true ), mNodeHelper( 0 ), mAllowAsync( false ),
    mSourceObject( 0 )
{
    mEngine = new Grantlee::Engine();

    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader());

    mEngine->addTemplateLoader( loader );
    loader->setTemplateDirs( QStringList() << KStandardDirs::locate("data","messageviewer/themes/") );
    // TODO: We should use KStandardDirs
    mEngine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH );
}

HeaderTheme::~HeaderTheme() {  
}

QString HeaderTheme::setTheming( const QString &themeName, KMime::Message *message ) const {
  
  Grantlee::Template t = mEngine->loadByName( themeName + ".html" );
  QVariantHash data;

  // Just for testing. Here should go the theme selected by the user
  QString strategy = "fancy";  

  // The direction of the header is determined according to the direction
  // of the application layout.

  QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
  data.insert( QLatin1String( "dir" ) , dir );

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

  //if ( showHeader( "subject" ) ) { This should be configured somewhere in the theme config file
    // the subject line and box below for details
    if ( strategy == "fancy" ) {
      const int flags = LinkLocator::PreserveSpaces |
                ( GlobalSettings::self()->showEmoticons() ?
                  LinkLocator::ReplaceSmileys : 0 );
      data.insert( QLatin1String( "subject" ) , strToHtml( message->subject()->asUnicodeString(), flags ) );
    }   
    else {
      data.insert( QLatin1String( "subject" ) , strToHtml( message->subject()->asUnicodeString()) );
    }  
    data.insert( QLatin1String( "subjectDir" ) , subjectDir );
   //}

  //if ( showHeader( "date" ) ) {
    if ( strategy == "brief" ) {
      data.insert( QLatin1String( "date" ) , strToHtml( dateString(message,isPrinting(),true ) ) );
    } else {
      data.insert( QLatin1String( "date" ) , strToHtml( dateString(message,isPrinting(),false ) ) );
    }  
    data.insert( QLatin1String( "dateDir" ), directionOf( dateStr( message->date()->dateTime() ) ) );
  //}

  // colors depend on if it is encapsulated or not
  QColor fontColor( Qt::white );
  QString linkColor = "white";
  const QColor activeColor = KColorScheme( QPalette::Active, KColorScheme::Selection ).
                                  background().color();
  QColor activeColorDark = activeColor.dark(130);
  // reverse colors for encapsulated
  if( !isTopLevel() ){
    activeColorDark = activeColor.dark(50);
    fontColor = QColor(Qt::black);
    linkColor = "black";
  }

  // 3D borders
  data.insert( QLatin1String( "activeColorDark" ), activeColorDark.name() );
  data.insert( QLatin1String( "fontColor" ), fontColor.name() );
  // kmail icon
  data.insert( QLatin1String( "isTopLevel" ) , "isTopLevel" );

  // Data managed only for the Enterprise Style
  if ( strategy == "brief" ) {
    data.insert( QLatin1String( "linkColor" ) , linkColor );

    if ( isPrinting() ) {
      //provide a bit more left padding when printing
      //kolab/issue3254 (printed mail cut at the left side)
      data.insert( QLatin1String( "isPrinting" ) , "isPrinting" );
    }
  }

  // Data only for the Fancy Style
  if ( strategy == "fancy" ) {
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

    if ( message->headerByType( "Resent-From" ) ) {
      data.insert( QLatin1String( "resentFrom" ), StringUtil::emailAddrAsAnchor( resentFrom, StringUtil::DisplayFullAddress ) );
    }

    if ( GlobalSettings::self()->showUserAgent() ) {
      //if ( showHeader( "user-agent" ) ) {
        if ( message->headerByType("User-Agent") ) {
          data.insert( QLatin1String( "user-agent" ), strToHtml( message->headerByType("User-Agent")->as7BitString() ) );
        }
      //}

      //if ( showHeader( "x-mailer" ) ) {
        if ( message->headerByType("X-Mailer") ) {
          data.insert( QLatin1String( "x-mailer" ), strToHtml( message->headerByType("X-Mailer")->as7BitString() ) );
        }
      //}
      
    }
  }    

  // from
  //if ( showHeader( "from" ) ) {
    if ( strategy == "brief" ) {
      data.insert( QLatin1String( "from" ) ,  StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly ) );  
    }
    else if ( strategy == "standard" ) {
      data.insert( QLatin1String( "from" ) , StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress, "", StringUtil::ShowLink ) );
    }
    else if ( strategy == "fancy" ) {
      data.insert( QLatin1String( "from" ), StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayFullAddress ) );
    }
    else {
      data.insert( QLatin1String( "from" ), StringUtil::emailAddrAsAnchor( message->from(), StringUtil::DisplayNameOnly, linkColor ));
    } 

    if ( !vCardName().isEmpty() )
      data.insert( QLatin1String( "vCardName" ) , vCardName() );
      
    //if ( showHeader( "organization" ) && message->headerByType("Organization"))
      data.insert( QLatin1String( "organization" ) , strToHtml(message->headerByType("Organization")->asUnicodeString()) );
  //}

  // to line
  //if ( showHeader( "to" ) ) {
    if ( strategy == "standard" ) {
      data.insert( QLatin1String( "to" ) , StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress ) );
    }
    else if ( strategy == "fancy" ) {
      data.insert( QLatin1String( "to" ), StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress,
                                                                     QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                                     "FullToAddressList",
                                                                     GlobalSettings::self()->numberOfAddressesToShow() ) );
    }
    else {
      data.insert( QLatin1String( "to" ) , StringUtil::emailAddrAsAnchor( message->to(), StringUtil::DisplayFullAddress, linkColor ) );
    }  
  //}  

  // cc line, if any
  //if ( showHeader( "cc" ) && message->cc( false ) ) {
    if ( strategy == "brief" ) {
      data.insert( QLatin1String( "cc" ) , StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayNameOnly ) );
    }
    else if ( strategy == "standard" ) {
      data.insert( QLatin1String( "cc" ) , StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress ) );
    }
    else if ( strategy == "fancy" ) {
      data.insert( QLatin1String( "cc" ), StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress,
                                                                       QString(), StringUtil::ShowLink, StringUtil::ExpandableAddresses,
                                                                       "FullCcAddressList",
                                                                       GlobalSettings::self()->numberOfAddressesToShow() ) );
    }
    else {
      data.insert( QLatin1String( "cc" ) , StringUtil::emailAddrAsAnchor( message->cc(), StringUtil::DisplayFullAddress, linkColor ) );
    }
  //} 

  // bcc line, if any
  //if ( showHeader( "bcc" ) && message->bcc( false ) ) {
    if ( strategy == "brief" ) {
      data.insert( QLatin1String( "bcc" ) , StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayNameOnly ) );
    }
    else if ( strategy == "standard" || strategy == "fancy" ) {
      data.insert( QLatin1String( "bcc" ) , StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress ) );
    }
    else {
      data.insert( QLatin1String( "bcc" ) , StringUtil::emailAddrAsAnchor( message->bcc(), StringUtil::DisplayFullAddress, linkColor ) );
    }
  //}    

  // Data managed only for the Plain Style
  if ( strategy == "standard" ) {
    //if ( strategy->headersToDisplay().isEmpty()
      //&& strategy->defaultPolicy() == HeaderStrategy::Display ) {
      // crude way to emulate "all" headers - Note: no strings have
      // i18n(), so direction should always be ltr.
      data.insert( QLatin1String( "message" ) , formatAllMessageHeaders( message ) );
    //}
  
    //if ( showHeader( "reply-to" ) && message->replyTo( false ) ) {
      data.insert( QLatin1String( "replyTo" ) , StringUtil::emailAddrAsAnchor( message->replyTo(), StringUtil::DisplayFullAddress ) );
    //}
  }

 // Data managed only for the Mobile Style
 
   // Bertjan: This is a highly simplified header for mobile devices. It lacks
  //          currently about almost everything except for the stuff that was in
  //          the mockup of Nuno. We might want to add additional items such as
  //          encryption I guess.

 
  if ( !messagePath().isEmpty() )
  {
    // TODO: Put these back in when we can somehow determine the path
    //headerStr += "  <td style=\"text-align: right; margin-right: 7px;\">" + i18n( "in:" )+ "</td>\n";
    data.insert( QLatin1String( "messagePath" ) , messagePath() );
  }

  QString spamHTML;

  if ( GlobalSettings::self()->showSpamStatus() ) {
    SpamScores scores = SpamHeaderAnalyzer::getSpamScores( message );
    for ( SpamScoresIterator it = scores.begin(); it != scores.end(); ++it )
      spamHTML += (*it).agent() + ' ' +
                   drawSpamMeter( (*it).error(), (*it).score(), (*it).confidence(), (*it).spamHeader(), (*it).confidenceHeader() );
  }

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
    data.insert( QLatin1String( "photoURL" ), photoURL );
    data.insert( QLatin1String( "photoWidth" ), photoWidth );
    data.insert( QLatin1String( "photoHeight" ), photoHeight );
    kDebug() << "Got a photo:" << photoURL;
  }

  if ( !spamHTML.isEmpty() ) {
    //data.insert( QLatin1String( "spamHTML" ), spamHTML );
    kDebug() << "Spam:" << spamHTML;
  }

  Grantlee::Context c( data );
  QString headerStr = t->render( &c );
  return headerStr;

}

QString HeaderTheme::formatAllMessageHeaders( KMime::Message *message ) const {
  QByteArray head = message->head();
  KMime::Headers::Generic *header = message->nextHeader(head);
  QString result;
  while ( header ) {
    result += strToHtml( header->asUnicodeString() );
    result += QLatin1String( "<br />\n" );
    delete header;
    header = message->nextHeader(head);
  }
  return result;
}

QString HeaderTheme::drawSpamMeter( SpamError spamError, double percent, double confidence,
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

HeaderTheme * HeaderTheme::create() {
  HeaderTheme * theme;
  theme = new HeaderTheme();
  return theme;
}

QString HeaderTheme::imgToDataUrl( const QImage &image )
{
  QByteArray ba;
  QBuffer buffer( &ba );
  buffer.open( QIODevice::WriteOnly );
  image.save( &buffer, "PNG" );
  return QString::fromLatin1("data:image/%1;base64,%2")
          .arg( QString::fromLatin1( "PNG" ), QString::fromLatin1( ba.toBase64() ) );
}

QString HeaderTheme::dateStr(const KDateTime &dateTime)
{
  const time_t unixTime = dateTime.toTime_t();
  return KMime::DateFormatter::formatDate(
              static_cast<KMime::DateFormatter::FormatType>(
                  MessageCore::GlobalSettings::self()->dateFormat() ),
              unixTime, MessageCore::GlobalSettings::self()->customDateFormat() );
}

QByteArray HeaderTheme::dateShortStr(const KDateTime &dateTime)
{
    const time_t unixTime = dateTime.toTime_t();

    QByteArray result = ctime(&unixTime);

    if (result[result.length()-1]=='\n')
        result.truncate(result.length()-1);

    return result;
}

}