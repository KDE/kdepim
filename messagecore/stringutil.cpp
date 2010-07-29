/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stringutil.h"

#include <kmime/kmime_charfreq.h>
#include <kmime/kmime_header_parsing.h>
#include <kmime/kmime_util.h>
#include <kmime/kmime_headers.h>
#include <kmime/kmime_message.h>
#include <KPIMUtils/Email>
#include <KPIMIdentities/IdentityManager>

#include <kascii.h>
#include <KConfigGroup>
#include <KDebug>
#include <KUser>
#include <KUrl>
#include <KDebug>

#include <QHostInfo>
#include <QRegExp>
#include <QStringList>
#include <kpimtextedit/textutils.h>

using namespace KMime;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

namespace MessageCore
{
namespace StringUtil
{

// Removes trailing spaces and tabs at the end of the line
static void removeTrailingSpace( QString &line )
{
  int i = line.length() - 1 ;
  while( ( i >= 0 ) && ( ( line[i] == ' ' ) || ( line[i] == '\t' ) ) )
    i--;
  line.truncate( i + 1 );
}

// Spilts the line off in two parts: The quote prefixes and the actual text of the line.
// For example, for the string "> > > Hello", it would be split up in "> > > " as the quote
// prefix, and "Hello" as the actual text.
// The actual text is written back to the "line" parameter, and the quote prefix is returned.
static QString splitLine( QString &line)
{
  removeTrailingSpace( line );
  int i = 0;
  int startOfActualText = -1;

  // TODO: Replace tabs with spaces first.

  // Loop through the chars in the line to find the place where the quote prefix stops
  while( i < line.length() )
  {
    const QChar c = line[i];
    const bool isAllowedQuoteChar = (c == '>') || (c == ':') || (c == '|') ||
                                    (c == ' ') || (c == '\t');
    if ( isAllowedQuoteChar )
      startOfActualText = i + 1;
    else
      break;
    i++;
  }

  // If the quote prefix only consists of whitespace, don't consider it as a quote prefix at all
  if ( line.left( startOfActualText ).trimmed().isEmpty() )
    startOfActualText = 0;

  // No quote prefix there -> nothing to do
  if ( startOfActualText <= 0 )
  {
    return QString();
  }

  // Entire line consists of only the quote prefix
  if ( i == line.length() )
  {
    const QString quotePrefix = line.left( startOfActualText );
    line.clear();
    return quotePrefix;
  }

  // Line contains both the quote prefix and the actual text, really split it up now
  const QString quotePrefix = line.left( startOfActualText );
  line = line.mid( startOfActualText );
  return quotePrefix;
}

// Writes all lines/text parts contained in the "textParts" list to the output text, "msg".
// Quote characters are added in front of each line, and no line is longer than
// maxLength.
//
// Although the lines in textParts are considered separate lines, they can actually be run
// together into a single line in some cases. This is basically the main difference to flowText().
//
// Example:
//   textParts = "Hello World, this is a test.", "Really"
//   indent = ">"
//   maxLength = 20
//   Result: "> Hello World, this\n
//            > is a test. Really"
// Notice how in this example, the text line "Really" is no longer a separate line, it was run
// together with a previously broken line.
//
// "textParts" is cleared upon return.
static bool flushPart( QString &msg, QStringList &textParts,
                       const QString &indent, int maxLength )
{
  maxLength -= indent.length();
  if ( maxLength < 20 )
    maxLength = 20;

  // Remove empty lines at end of quote
  while ( !textParts.isEmpty() && textParts.last().isEmpty() ) {
    textParts.removeLast();
  }

  QString text;
  foreach( const QString line, textParts ) {

    // An empty line in the input means that an empty line should be in the output as well.
    // Therefore, we write all of our text so far to the msg.
    if ( line.isEmpty() ) {
      if ( !text.isEmpty() )
        msg += KPIMTextEdit::TextUtils::flowText( text, indent, maxLength );
      msg += indent + '\n';
    }

    else {
      if ( text.isEmpty() )
        text = line;
      else
        text += ' ' + line.trimmed();

      // If the line doesn't need to be wrapped at all, just write it out as-is.
      // When a line exceeds the maximum length and therefore needs to be broken, this statement
      // if false, and therefore we keep adding lines to our text, so they get ran together in the
      // next flowText call, as "text" contains several text parts/lines then.
      if ( ( text.length() < maxLength ) || ( line.length() < ( maxLength - 10 ) ) )
        msg += KPIMTextEdit::TextUtils::flowText( text, indent, maxLength );
    }
  }

  // Write out pending text to the msg
  if ( !text.isEmpty() )
    msg += KPIMTextEdit::TextUtils::flowText( text, indent, maxLength );

  const bool appendEmptyLine = !textParts.isEmpty();
  textParts.clear();
  return appendEmptyLine;
}

QStringList stripMyAddressesFromAddressList( const QStringList& list, const KPIMIdentities::IdentityManager* identMan )
{
  QStringList addresses = list;
  for( QStringList::Iterator it = addresses.begin();
       it != addresses.end(); ) {
    kDebug() << "Check whether" << *it <<"is one of my addresses";
    if( identMan->thatIsMe( KPIMUtils::extractEmailAddress( *it ) ) ) {
      kDebug() << "Removing" << *it <<"from the address list";
      it = addresses.erase( it );
    }
    else
      ++it;
  }
  return addresses;
}

QMap<QString, QString> parseMailtoUrl ( const KUrl& url )
{
  kDebug() << url.pathOrUrl();
  QMap<QString, QString> values = url.queryItems( KUrl::CaseInsensitiveKeys );
  QString to = KPIMUtils::decodeMailtoUrl( url );
  to = to.isEmpty() ?  values.value( "to" ) : to + QString( ", " ) + values.value( "to" );
  values.insert( "to", to );
  return values;
}

QString stripSignature ( const QString & msg, bool clearSigned )
{
  // Following RFC 3676, only > before --
  // I prefer to not delete a SB instead of delete good mail content.
  const QRegExp sbDelimiterSearch = clearSigned ?
      QRegExp( "(^|\n)[> ]*--\\s?\n" ) : QRegExp( "(^|\n)[> ]*-- \n" );
  // The regular expression to look for prefix change
  const QRegExp commonReplySearch = QRegExp( "^[ ]*>" );

  QString res = msg;
  int posDeletingStart = 1; // to start looking at 0

  // While there are SB delimiters (start looking just before the deleted SB)
  while ( ( posDeletingStart = res.indexOf( sbDelimiterSearch , posDeletingStart -1 ) ) >= 0 )
  {
    QString prefix; // the current prefix
    QString line; // the line to check if is part of the SB
    int posNewLine = -1;
    int posSignatureBlock = -1;
    // Look for the SB beginning
    posSignatureBlock = res.indexOf( '-', posDeletingStart );
    // The prefix before "-- "$
    if ( res[posDeletingStart] == '\n' ) ++posDeletingStart;
    prefix = res.mid( posDeletingStart, posSignatureBlock - posDeletingStart );
    posNewLine = res.indexOf( '\n', posSignatureBlock ) + 1;

    // now go to the end of the SB
    while ( posNewLine < res.size() && posNewLine > 0 )
    {
      // handle the undefined case for mid ( x , -n ) where n>1
      int nextPosNewLine = res.indexOf( '\n', posNewLine );
      if ( nextPosNewLine < 0 ) nextPosNewLine = posNewLine - 1;
      line = res.mid( posNewLine, nextPosNewLine - posNewLine );

      // check when the SB ends:
      // * does not starts with prefix or
      // * starts with prefix+(any substring of prefix)
      if ( ( prefix.isEmpty() && line.indexOf( commonReplySearch ) < 0 ) ||
             ( !prefix.isEmpty() && line.startsWith( prefix ) &&
             line.mid( prefix.size() ).indexOf( commonReplySearch ) < 0 ) )
      {
        posNewLine = res.indexOf( '\n', posNewLine ) + 1;
      }
      else
        break; // end of the SB
    }
    // remove the SB or truncate when is the last SB
    if ( posNewLine > 0 )
      res.remove( posDeletingStart, posNewLine - posDeletingStart );
    else
      res.truncate( posDeletingStart );
  }
  return res;
}

AddressList splitAddrField( const QByteArray & str )
{
  AddressList result;
  const char * scursor = str.begin();
  if ( !scursor )
    return AddressList();
  const char * const send = str.begin() + str.length();
  if ( !parseAddressList( scursor, send, result ) )
    kDebug() << "Error in address splitting: parseAddressList returned false!";
  return result;
}

QString generateMessageId( const QString& addr, const QString &msgIdSuffix )
{
  const QDateTime datetime = QDateTime::currentDateTime();

  QString msgIdStr = '<' + datetime.toString( "yyyyMMddhhmm.sszzz" );

  if( !msgIdSuffix.isEmpty() )
    msgIdStr += '@' + msgIdSuffix;
  else
    msgIdStr += '.' + KPIMUtils::toIdn( addr );

  msgIdStr += '>';

  return msgIdStr;
}

QByteArray html2source( const QByteArray & src )
{
  QByteArray result( 1 + 6*src.length(), '\0' );  // maximal possible length

  QByteArray::ConstIterator s = src.begin();
  QByteArray::Iterator d = result.begin();
  while ( *s ) {
    switch ( *s ) {
      case '<': {
        *d++ = '&';
        *d++ = 'l';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
      case '\r': {
        ++s;
      }
      break;
      case '\n': {
        *d++ = '<';
        *d++ = 'b';
        *d++ = 'r';
        *d++ = '>';
        ++s;
      }
      break;
      case '>': {
        *d++ = '&';
        *d++ = 'g';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
      case '&': {
        *d++ = '&';
        *d++ = 'a';
        *d++ = 'm';
        *d++ = 'p';
        *d++ = ';';
        ++s;
      }
      break;
      case '"': {
        *d++ = '&';
        *d++ = 'q';
        *d++ = 'u';
        *d++ = 'o';
        *d++ = 't';
        *d++ = ';';
        ++s;
      }
      break;
      case '\'': {
        *d++ = '&';
        *d++ = 'a';
        *d++ = 'p';
        *d++ = 's';
        *d++ = ';';
        ++s;
      }
      break;
      default:
        *d++ = *s++;
    }
  }
  result.truncate( d - result.begin() );
  return result;
}

QByteArray stripEmailAddr( const QByteArray& aStr )
{
  if ( aStr.isEmpty() )
    return QByteArray();

  QByteArray result;

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  QByteArray name;
  QByteArray comment;
  QByteArray angleAddress;
  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;

  for ( const char* p = aStr.data(); *p; ++p ) {
    switch ( context ) {
      case TopLevel : {
        switch ( *p ) {
          case '"' : inQuotedString = !inQuotedString;
          break;
          case '(' : if ( !inQuotedString ) {
            context = InComment;
            commentLevel = 1;
          }
          else
            name += *p;
          break;
          case '<' : if ( !inQuotedString ) {
            context = InAngleAddress;
          }
          else
            name += *p;
          break;
          case '\\' : // quoted character
            ++p; // skip the '\'
            if ( *p )
              name += *p;
            break;
            case ',' : if ( !inQuotedString ) {
                   // next email address
              if ( !result.isEmpty() )
                result += ", ";
              name = name.trimmed();
              comment = comment.trimmed();
              angleAddress = angleAddress.trimmed();
              if ( angleAddress.isEmpty() && !comment.isEmpty() ) {
                     // handle Outlook-style addresses like
                     // john.doe@invalid (John Doe)
                result += comment;
              }
              else if ( !name.isEmpty() ) {
                result += name;
              }
              else if ( !comment.isEmpty() ) {
                result += comment;
              }
              else if ( !angleAddress.isEmpty() ) {
                result += angleAddress;
              }
              name = QByteArray();
              comment = QByteArray();
              angleAddress = QByteArray();
            }
            else
              name += *p;
            break;
            default :  name += *p;
        }
        break;
      }
      case InComment : {
        switch ( *p ) {
          case '(' : ++commentLevel;
          comment += *p;
          break;
          case ')' : --commentLevel;
          if ( commentLevel == 0 ) {
            context = TopLevel;
            comment += ' '; // separate the text of several comments
          }
          else
            comment += *p;
          break;
          case '\\' : // quoted character
            ++p; // skip the '\'
            if ( *p )
              comment += *p;
            break;
            default :  comment += *p;
        }
        break;
      }
      case InAngleAddress : {
        switch ( *p ) {
          case '"' : inQuotedString = !inQuotedString;
          angleAddress += *p;
          break;
          case '>' : if ( !inQuotedString ) {
            context = TopLevel;
          }
          else
            angleAddress += *p;
          break;
          case '\\' : // quoted character
            ++p; // skip the '\'
            if ( *p )
              angleAddress += *p;
            break;
            default :  angleAddress += *p;
        }
        break;
      }
    } // switch ( context )
  }
  if ( !result.isEmpty() )
    result += ", ";
  name = name.trimmed();
  comment = comment.trimmed();
  angleAddress = angleAddress.trimmed();
  if ( angleAddress.isEmpty() && !comment.isEmpty() ) {
    // handle Outlook-style addresses like
    // john.doe@invalid (John Doe)
    result += comment;
  }
  else if ( !name.isEmpty() ) {
    result += name;
  }
  else if ( !comment.isEmpty() ) {
    result += comment;
  }
  else if ( !angleAddress.isEmpty() ) {
    result += angleAddress;
  }

  //kDebug() << "Returns \"" << result << "\"";
  return result;
}

QString stripEmailAddr( const QString& aStr )
{
  //kDebug() << "(" << aStr << ")";

  if ( aStr.isEmpty() )
    return QString();

  QString result;

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  QString name;
  QString comment;
  QString angleAddress;
  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;

  QChar ch;
  int strLength(aStr.length());
  for ( int index = 0; index < strLength; ++index ) {
    ch = aStr[index];
    switch ( context ) {
      case TopLevel : {
        switch ( ch.toLatin1() ) {
          case '"' : inQuotedString = !inQuotedString;
          break;
          case '(' : if ( !inQuotedString ) {
            context = InComment;
            commentLevel = 1;
          }
          else
            name += ch;
          break;
          case '<' : if ( !inQuotedString ) {
            context = InAngleAddress;
          }
          else
            name += ch;
          break;
          case '\\' : // quoted character
            ++index; // skip the '\'
            if ( index < aStr.length() )
              name += aStr[index];
            break;
            case ',' : if ( !inQuotedString ) {
                   // next email address
              if ( !result.isEmpty() )
                result += ", ";
              name = name.trimmed();
              comment = comment.trimmed();
              angleAddress = angleAddress.trimmed();
              if ( angleAddress.isEmpty() && !comment.isEmpty() ) {
                     // handle Outlook-style addresses like
                     // john.doe@invalid (John Doe)
                result += comment;
              }
              else if ( !name.isEmpty() ) {
                result += name;
              }
              else if ( !comment.isEmpty() ) {
                result += comment;
              }
              else if ( !angleAddress.isEmpty() ) {
                result += angleAddress;
              }
              name.clear();
              comment.clear();
              angleAddress.clear();
            }
            else
              name += ch;
            break;
            default :  name += ch;
        }
        break;
      }
      case InComment : {
        switch ( ch.toLatin1() ) {
          case '(' : ++commentLevel;
          comment += ch;
          break;
          case ')' : --commentLevel;
          if ( commentLevel == 0 ) {
            context = TopLevel;
            comment += ' '; // separate the text of several comments
          }
          else
            comment += ch;
          break;
          case '\\' : // quoted character
            ++index; // skip the '\'
            if ( index < aStr.length() )
              comment += aStr[index];
            break;
            default :  comment += ch;
        }
        break;
      }
      case InAngleAddress : {
        switch ( ch.toLatin1() ) {
          case '"' : inQuotedString = !inQuotedString;
          angleAddress += ch;
          break;
          case '>' : if ( !inQuotedString ) {
            context = TopLevel;
          }
          else
            angleAddress += ch;
          break;
          case '\\' : // quoted character
            ++index; // skip the '\'
            if ( index < aStr.length() )
              angleAddress += aStr[index];
            break;
            default :  angleAddress += ch;
        }
        break;
      }
    } // switch ( context )
  }
  if ( !result.isEmpty() )
    result += ", ";
  name = name.trimmed();
  comment = comment.trimmed();
  angleAddress = angleAddress.trimmed();
  if ( angleAddress.isEmpty() && !comment.isEmpty() ) {
    // handle Outlook-style addresses like
    // john.doe@invalid (John Doe)
    result += comment;
  }
  else if ( !name.isEmpty() ) {
    result += name;
  }
  else if ( !comment.isEmpty() ) {
    result += comment;
  }
  else if ( !angleAddress.isEmpty() ) {
    result += angleAddress;
  }

  //kDebug() << "Returns \"" << result << "\"";
  return result;
}

QString quoteHtmlChars( const QString& str, bool removeLineBreaks )
{
  QString result;

  unsigned int strLength(str.length());
  result.reserve( 6*strLength ); // maximal possible length
  for( unsigned int i = 0; i < strLength; ++i ) {
    switch ( str[i].toLatin1() ) {
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '&':
        result += "&amp;";
        break;
      case '"':
        result += "&quot;";
        break;
      case '\n':
        if ( !removeLineBreaks )
          result += "<br>";
        break;
      case '\r':
      // ignore CR
        break;
      default:
        result += str[i];
    }
  }

  result.squeeze();
  return result;
}

void removePrivateHeaderFields( const KMime::Message::Ptr &msg ) {
  msg->removeHeader("Status");
  msg->removeHeader("X-Status");
  msg->removeHeader("X-KMail-EncryptionState");
  msg->removeHeader("X-KMail-SignatureState");
  msg->removeHeader("X-KMail-MDN-Sent");
  msg->removeHeader("X-KMail-Transport");
  msg->removeHeader("X-KMail-Identity");
  msg->removeHeader("X-KMail-Fcc");
  msg->removeHeader("X-KMail-Redirect-From");
  msg->removeHeader("X-KMail-Link-Message");
  msg->removeHeader("X-KMail-Link-Type");
  msg->removeHeader("X-KMail-QuotePrefix");
  msg->removeHeader("X-KMail-CursorPos");
  msg->removeHeader( "X-KMail-Templates" );
  msg->removeHeader( "X-KMail-Drafts" );
  msg->removeHeader( "X-KMail-Tag" );
}

QByteArray asSendableString( const KMime::Message::Ptr &msg )
{
  KMime::Message::Ptr message( new KMime::Message );
  message->setContent( msg->encodedContent() );
  removePrivateHeaderFields( message );
  message->removeHeader("Bcc");
  return message->encodedContent();
}

QByteArray headerAsSendableString( const KMime::Message::Ptr &msg )
{
  KMime::Message::Ptr message( new KMime::Message );
  message->setContent( msg->encodedContent() );
  removePrivateHeaderFields( message );
  message->removeHeader("Bcc");
  return message->head();
}


QString emailAddrAsAnchor( const KMime::Types::Mailbox::List &mailboxList,
                           Display display, const QString& cssStyle,
                           Link link, AddressMode expandable, const QString& fieldName,
                           int collapseNumber )
{
  QString result;
  int numberAddresses = 0;
  bool expandableInserted = false;


  foreach( KMime::Types::Mailbox mailbox, mailboxList ) {
    if( !mailbox.prettyAddress().isEmpty() ) {
      numberAddresses++;
      if( expandable == ExpandableAddresses && !expandableInserted && numberAddresses > collapseNumber ) {
        result = "<span id=\"icon" + fieldName + "\"></span>" + result;
        result += "<span id=\"dots" + fieldName + "\">...</span><span id=\"hidden" + fieldName +"\">";
        expandableInserted = true;
      }

      if( link == ShowLink ) {
        result += "<a href=\"mailto:"
                + KUrl::toPercentEncoding( KPIMUtils::encodeMailtoUrl( mailbox.prettyAddress( KMime::Types::Mailbox::QuoteWhenNecessary ) ).path() )
                + "\" "+cssStyle+">";
      }
      if ( display == DisplayNameOnly ) {
        if ( !mailbox.name().isEmpty() ) // Fallback to the email address when the name is not set.
          result += quoteHtmlChars( mailbox.name(), true );
        else
          result += quoteHtmlChars( mailbox.prettyAddress(), true );
      } else {
        result += quoteHtmlChars( mailbox.prettyAddress( KMime::Types::Mailbox::QuoteWhenNecessary ), true );
      }
      if( link == ShowLink ) {
        result += "</a>, ";
      }
    }
  }

  // cut of the trailing ", "
  if( link == ShowLink ) {
    result.truncate( result.length() - 2 );
  }

  if( expandableInserted ) {
    result += "</span>";
  }
  return result;
}

QString emailAddrAsAnchor( KMime::Headers::Generics::MailboxList *mailboxList,
                           Display display, const QString& cssStyle,
                           Link link, AddressMode expandable, const QString& fieldName,
                           int collapseNumber )
{
  Q_ASSERT( mailboxList );
  return emailAddrAsAnchor( mailboxList->mailboxes(), display, cssStyle, link, expandable, fieldName, collapseNumber );
}

QString emailAddrAsAnchor( KMime::Headers::Generics::AddressList *addressList,
                           Display display, const QString& cssStyle,
                           Link link, AddressMode expandable, const QString& fieldName,
                           int collapseNumber )
{
  Q_ASSERT( addressList );
  return emailAddrAsAnchor( addressList->mailboxes(), display, cssStyle, link, expandable, fieldName, collapseNumber );
}

QStringList stripAddressFromAddressList( const QString& address,
                                         const QStringList& list )
{
  QStringList addresses( list );
  QString addrSpec( KPIMUtils::extractEmailAddress( address ) );
  for ( QStringList::Iterator it = addresses.begin();
        it != addresses.end(); ) {
    if ( kasciistricmp( addrSpec.toUtf8().data(),
         KPIMUtils::extractEmailAddress( *it ).toUtf8().data() ) == 0 ) {
      kDebug() << "Removing" << *it << "from the address list";
      it = addresses.erase( it );
    }
    else
      ++it;
  }
  return addresses;
}

bool addressIsInAddressList( const QString& address,
                             const QStringList& addresses )
{
  QString addrSpec = KPIMUtils::extractEmailAddress( address );
  for( QStringList::ConstIterator it = addresses.begin();
       it != addresses.end(); ++it ) {
    if ( kasciistricmp( addrSpec.toUtf8().data(),
         KPIMUtils::extractEmailAddress( *it ).toUtf8().data() ) == 0 )
      return true;
  }
  return false;
}

QString guessEmailAddressFromLoginName( const QString& loginName )
{
  if ( loginName.isEmpty() )
    return QString();

  QString address = loginName;
  address += '@';
  address += QHostInfo::localHostName();

  // try to determine the real name
  const KUser user( loginName );
  if ( user.isValid() ) {
    QString fullName = user.property( KUser::FullName ).toString();
    if ( fullName.contains( QRegExp( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" ) ) )
      address = '"' + fullName.replace( '\\', "\\" ).replace( '"', "\\" )
          + "\" <" + address + '>';
    else
      address = fullName + " <" + address + '>';
  }

  return address;
}

QString smartQuote( const QString &msg, int maxLineLength )
{
  // The algorithm here is as follows:
  // We split up the incoming msg into lines, and then iterate over each line.
  // We keep adding lines with the same indent ( = quote prefix, e.g. "> " ) to a
  // "textParts" list. So the textParts list contains only lines with the same quote
  // prefix.
  //
  // When all lines with the same indent are collected in "textParts", we write those out
  // to the result by calling flushPart(), which does all the nice formatting for us.

  QStringList textParts;
  QString oldIndent;
  bool firstPart = true;
  QString result;
  foreach ( QString line, msg.split( '\n' ) ) {

    // Split off the indent from the line
    const QString indent = splitLine( line );

    if ( line.isEmpty() ) {
      if ( !firstPart )
        textParts.append( QString() );
      continue;
    };

    if ( firstPart ) {
      oldIndent = indent;
      firstPart = false;
    }

    // The indent changed, that means we have to write everything contained in textParts to the
    // result, which we do by calling flushPart().
    if ( oldIndent != indent ) {

      // Check if the last non-blank line is a "From" line. A from line is the line containing the
      // attribution to a quote, e.g. "Yesterday, you wrote:". We'll just check for the last colon
      // here, to simply things.
      // If there is a From line, remove it from the textParts to that flushPart won't break it.
      // We'll manually add it to the result afterwards.
      QString fromLine;
      if ( !textParts.isEmpty() ) {
        for ( int i = textParts.count() - 1; i >= 0; i-- ) {

          // Check if we have found the From line
          if ( textParts[i].endsWith( ':' ) ) {
            fromLine = oldIndent + textParts[i] + '\n';
            textParts.removeAt( i );
            break;
          }

          // Abort on first non-empty line
          if ( !textParts[i].trimmed().isEmpty() )
            break;
        }
      }

      // Write out all lines with the same indent using flushPart(). The textParts list
      // is cleared for us.
      if ( flushPart( result, textParts, oldIndent, maxLineLength ) ) {
        if ( oldIndent.length() > indent.length() )
          result += indent + '\n';
        else
          result += oldIndent + '\n';
      }

      if ( !fromLine.isEmpty() ) {
        result += fromLine;
      }

      oldIndent = indent;
    }

    textParts.append( line );
  }

  // Write out anything still pending
  flushPart( result, textParts, oldIndent, maxLineLength );

  // Remove superfluous newline which was appended in flowText
  if ( !result.isEmpty() && result.endsWith( '\n' ) )
    result.chop( 1 );

  return result;
}

bool isCryptoPart( const QString &type, const QString &subType, const QString &fileName )
{
  return ( type.toLower() == "application" &&
           ( subType.toLower() == "pgp-encrypted" ||
             subType.toLower() == "pgp-signature" ||
             subType.toLower() == "pkcs7-mime" ||
             subType.toLower() == "pkcs7-signature" ||
             subType.toLower() == "x-pkcs7-signature" ||
             ( subType.toLower() == "octet-stream" &&
               fileName.toLower() == "msg.asc" ) ) );
}

QString formatString( const QString &wildString, const QString &fromAddr )
{
  QString result;

  if ( wildString.isEmpty() ) {
    return wildString;
  }

  unsigned int strLength( wildString.length() );
  for ( uint i=0; i<strLength; ) {
    QChar ch = wildString[i++];
    if ( ch == '%' && i<strLength ) {
      ch = wildString[i++];
      switch ( ch.toLatin1() ) {
      case 'f': // sender's initals
      {
        QString str = stripEmailAddr( fromAddr );

        uint j = 0;
        for ( ; str[j]>' '; j++ )
          ;
        unsigned int strLength( str.length() );
        for ( ; j < strLength && str[j] <= ' '; j++ )
          ;
        result += str[0];
        if ( str[j] > ' ' ) {
          result += str[j];
        } else {
          if ( str[1] > ' ' ) {
            result += str[1];
          }
        }
      }
      break;
      case '_':
        result += ' ';
        break;
      case '%':
        result += '%';
        break;
      default:
        result += '%';
        result += ch;
        break;
      }
    } else {
      result += ch;
    }
  }
  return result;
}

QString cleanFileName( const QString &name )
{
  QString fileName = name.trimmed();

  // We need to replace colons with underscores since those cause problems with
  // KFileDialog (bug in KFileDialog though) and also on Windows filesystems.
  // We also look at the special case of ": ", since converting that to "_ "
  // would look strange, simply "_" looks better.
  // https://issues.kolab.org/issue3805
  fileName.replace( ": ", "_" );
  // replace all ':' with '_' because ':' isn't allowed on FAT volumes
  fileName.replace( ':', '_' );
  // better not use a dir-delimiter in a filename
  fileName.replace( '/', '_' );
  fileName.replace( '\\', '_' );

  // replace all '.' with '_', not just at the start of the filename
  // but don't replace the last '.' before the file extension.
  int i = fileName.lastIndexOf( '.' );
  if( i != -1 )
    i = fileName.lastIndexOf( '.' , i - 1 );
  
  while( i != -1 )
  {
    fileName.replace( i, 1, '_' );
    i = fileName.lastIndexOf( '.', i - 1 );
  }
  
  // replace all '~' with '_', not just leading '~' either.
  fileName.replace( '~', '_' );

  return fileName;
}

}


}
