/*
    kmime_util.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmime_util.h"

#include <kcodecs.h> // for KCodec::{quotedPrintableDe,base64{En,De}}code
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>

#include <qtextcodec.h>
#include <q3strlist.h> // for QStrIList
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

#include <stdlib.h>
#include <ctype.h>
#include <time.h> // for time()
#include <unistd.h> // for getpid()

using namespace KMime;

namespace KMime {

Q3StrIList c_harsetCache;
Q3StrIList l_anguageCache;

const char* cachedCharset(const Q3CString &name)
{
  int idx=c_harsetCache.find(name.data());
  if(idx>-1)
    return c_harsetCache.at(idx);

  c_harsetCache.append(name.upper().data());
  //kdDebug() << "KNMimeBase::cachedCharset() number of cs " << c_harsetCache.count() << endl;
  return c_harsetCache.last();
}

const char* cachedLanguage(const Q3CString &name)
{
  int idx=l_anguageCache.find(name.data());
  if(idx>-1)
    return l_anguageCache.at(idx);

  l_anguageCache.append(name.upper().data());
  //kdDebug() << "KNMimeBase::cachedCharset() number of cs " << c_harsetCache.count() << endl;
  return l_anguageCache.last();
}

bool isUsAscii(const QString &s)
{
  uint sLength = s.length();
  for (uint i=0; i<sLength; i++)
    if (s.at(i).latin1()<=0)    // c==0: non-latin1, c<0: non-us-ascii
      return false;

  return true;
}

// "(),.:;<>@[\]
const uchar specialsMap[16] = {
  0x00, 0x00, 0x00, 0x00, // CTLs
  0x20, 0xCA, 0x00, 0x3A, // SPACE ... '?'
  0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
  0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// "(),:;<>@[\]/=?
const uchar tSpecialsMap[16] = {
  0x00, 0x00, 0x00, 0x00, // CTLs
  0x20, 0xC9, 0x00, 0x3F, // SPACE ... '?'
  0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
  0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// all except specials, CTLs, SPACE.
const uchar aTextMap[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x5F, 0x35, 0xFF, 0xC5,
  0x7F, 0xFF, 0xFF, 0xE3,
  0xFF, 0xFF, 0xFF, 0xFE
};

// all except tspecials, CTLs, SPACE.
const uchar tTextMap[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x5F, 0x36, 0xFF, 0xC0,
  0x7F, 0xFF, 0xFF, 0xE3,
  0xFF, 0xFF, 0xFF, 0xFE
};

// none except a-zA-Z0-9!*+-/
const uchar eTextMap[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x40, 0x35, 0xFF, 0xC0,
  0x7F, 0xFF, 0xFF, 0xE0,
  0x7F, 0xFF, 0xFF, 0xE0
};

#if defined(_AIX) && defined(truncate)
#undef truncate
#endif

QString decodeRFC2047String(const Q3CString &src, const char **usedCS,
			    const Q3CString &defaultCS, bool forceCS)
{
  Q3CString result, str;
  Q3CString declaredCS;
  int pos = 0, dest = 0, beg = 0, end = 0, mid = 0, endOfLastEncWord = 0;
  char encoding = '\0';
  bool valid, onlySpacesSinceLastWord=false;
  const int maxLen=400;
  int i;

  if ( !src.contains( "=?" ) )
    result = src.copy();
  else {
    result.truncate(src.length());
    for (pos = 0, dest = 0; pos < src.size(); pos++)
    {
      if ( src[pos] != '=' || src[pos + 1] != '?' )
      {
        result[dest++] = src[pos];
        if (onlySpacesSinceLastWord)
          onlySpacesSinceLastWord = (src[pos]==' ' || src[pos]=='\t');
        continue;
      }
      beg = pos+2;
      end = beg;
      valid = true;
      // parse charset name
      declaredCS="";
      for ( i = 2, pos += 2; i < maxLen && (src[pos] != '?' && (ispunct(src[pos]) || isalnum(src[pos]))); i++ ) {
        declaredCS += src[pos];
        pos++;
      }
      if ( src[pos] != '?' || i < 4 || i >= maxLen) valid = false;
      else
      {
        // get encoding and check delimiting question marks
        encoding = toupper(src[pos+1]);
        if ( src[pos+2] != '?' || (encoding != 'Q' && encoding != 'B'))
          valid = false;
        pos += 3;
        i+=3;
      }
      if (valid)
      {
        mid = pos;
        // search for end of encoded part
        while ( i < maxLen && pos < src.size() && ! ( src[pos] == '?' && src[pos + 1] == '=' ) )
        {
          i++;
          pos++;
        }
        end = pos+2;//end now points to the first char after the encoded string
        if ( i >= maxLen || src.size() <= pos ) valid = false;
      }

      if (valid) {
        // cut all linear-white space between two encoded words
        if (onlySpacesSinceLastWord)
          dest=endOfLastEncWord;

        if (mid < pos) {
          str = src.mid( mid, (int)(pos - mid + 1) );
          if (encoding == 'Q')
          {
            // decode quoted printable text
            for (i=str.length()-1; i>=0; i--)
              if (str[i]=='_') str[i]=' ';
            str = KCodecs::quotedPrintableDecode(str);
          }
          else
          {
            str = KCodecs::base64Decode(str);
          }
          for (i=0; str[i]; i++) {
            result[dest++] = str[i];
          }
        }

        endOfLastEncWord=dest;
        onlySpacesSinceLastWord=true;

        pos = end -1;
      }
      else
      {
        pos = beg - 2;
        result[dest++] = src[pos++];
        result[dest++] = src[pos];
      }
    }
    result[dest] = '\0';
  }

  //find suitable QTextCodec
  QTextCodec *codec=0;
  bool ok=true;
  if (forceCS || declaredCS.isEmpty()) {
    codec=KGlobal::charsets()->codecForName(defaultCS);
    (*usedCS)=cachedCharset(defaultCS);
  }
  else {
    codec=KGlobal::charsets()->codecForName(declaredCS, ok);
    if(!ok) {     //no suitable codec found => use default charset
      codec=KGlobal::charsets()->codecForName(defaultCS);
      (*usedCS)=cachedCharset(defaultCS);
    }
    else
      (*usedCS)=cachedCharset(declaredCS);
  }

  return codec->toUnicode(result.data(), result.length());
}


Q3CString encodeRFC2047String(const QString &src, const char *charset,
			     bool addressHeader, bool allow8BitHeaders)
{
  Q3CString encoded8Bit, result, usedCS;
  int start=0,end=0;
  bool nonAscii=false, ok=true, useQEncoding=false;
  QTextCodec *codec=0;

  usedCS=charset;
  codec=KGlobal::charsets()->codecForName(usedCS, ok);

  if(!ok) {
    //no codec available => try local8Bit and hope the best ;-)
    usedCS=KGlobal::locale()->encoding();
    codec=KGlobal::charsets()->codecForName(usedCS, ok);
  }

  if (usedCS.find("8859-")>=0)  // use "B"-Encoding for non iso-8859-x charsets
    useQEncoding=true;

  encoded8Bit=codec->fromUnicode(src);

  if(allow8BitHeaders)
    return encoded8Bit;

  uint encoded8BitLength = encoded8Bit.length();
  for (unsigned int i=0; i<encoded8BitLength; i++) {
    if (encoded8Bit[i]==' ')    // encoding starts at word boundaries
      start = i+1;

    // encode escape character, for japanese encodings...
    if (((signed char)encoded8Bit[i]<0) || (encoded8Bit[i] == '\033') ||
        (addressHeader && (strchr("\"()<>@,.;:\\[]=",encoded8Bit[i])!=0))) {
      end = start;   // non us-ascii char found, now we determine where to stop encoding
      nonAscii=true;
      break;
    }
  }

  if (nonAscii) {
    while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
      end++;

    for (int x=end;x<encoded8Bit.length();x++)
      if (((signed char)encoded8Bit[x]<0) || (encoded8Bit[x] == '\033') ||
          (addressHeader && (strchr("\"()<>@,.;:\\[]=",encoded8Bit[x])!=0))) {
        end = encoded8Bit.length();     // we found another non-ascii word

      while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
        end++;
    }

    result = encoded8Bit.left(start)+"=?"+usedCS;

    if (useQEncoding) {
      result += "?Q?";

      char c,hexcode;                       // implementation of the "Q"-encoding described in RFC 2047
      for (int i=start;i<end;i++) {
        c = encoded8Bit[i];
        if (c == ' ')       // make the result readable with not MIME-capable readers
          result+='_';
        else
          if (((c>='a')&&(c<='z'))||      // paranoid mode, we encode *all* special characters to avoid problems
              ((c>='A')&&(c<='Z'))||      // with "From" & "To" headers
              ((c>='0')&&(c<='9')))
            result+=c;
          else {
            result += "=";                 // "stolen" from KMail ;-)
            hexcode = ((c & 0xF0) >> 4) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
            hexcode = (c & 0x0F) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
          }
      }
    } else {
      result += "?B?"+KCodecs::base64Encode(encoded8Bit.mid(start,end-start), false);
    }

    result +="?=";
    result += encoded8Bit.right(encoded8Bit.length()-end);
  }
  else
    result = encoded8Bit;

  return result;
}

Q3CString uniqueString()
{
  static char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  time_t now;
  Q3CString ret;
  char p[11];
  int pos, ran;
  unsigned int timeval;

  p[10]='\0';
  now=time(0);
  ran=1+(int) (1000.0*rand()/(RAND_MAX+1.0));
  timeval=(now/ran)+getpid();

  for(int i=0; i<10; i++){
    pos=(int) (61.0*rand()/(RAND_MAX+1.0));
    //kdDebug(5003) << pos << endl;
    p[i]=chars[pos];
  }
  ret.sprintf("%d.%s", timeval, p);

  return ret;
}


Q3CString multiPartBoundary()
{
  Q3CString ret;
  ret="nextPart"+uniqueString();
  return ret;
}

Q3CString extractHeader(const Q3CString &src, const char *name)
{
  Q3CString n=Q3CString(name)+": ";
  int pos1=-1, pos2=0, len=src.length()-1;
  bool folded(false);

  if (n.lower() == src.left(n.length()).lower()) {
    pos1 = 0;
  } else {
    n.prepend("\n");
    pos1 = QString(src).indexOf(n,0,Qt::CaseInsensitive);
  }

  if (pos1>-1) {    //there is a header with the given name
    pos1+=n.length(); //skip the name
    pos2=pos1;

    if (src[pos2]!='\n') {  // check if the header is not empty
      while(1) {
        pos2=src.find("\n", pos2+1);
        if(pos2==-1 || pos2==len || ( src[pos2+1]!=' ' && src[pos2+1]!='\t') ) //break if we reach the end of the string, honor folded lines
          break;
        else
          folded = true;
      }
    }

    if(pos2<0) pos2=len+1; //take the rest of the string

    if (!folded)
      return src.mid(pos1, pos2-pos1);
    else {
      QByteArray hdrValue = src.mid( pos1, pos2 - pos1 );
      // unfold header
      int beg = 0, mid = 0, end = 0;
      while ( (mid = hdrValue.indexOf( '\n' )) >= 0 ) {
        beg = end = mid;
        while ( beg > 0 ) {
          if ( !QChar( hdrValue[beg] ).isSpace() ) break;
          --beg;
        }
        while ( end < hdrValue.length() - 1 ) {
          if ( !QChar( hdrValue[end] ).isSpace() ) break;
          ++end;
        }
        hdrValue.remove( beg, end - beg );
      }
      return hdrValue; 
    }
  }
  else {
    return Q3CString(0); //header not found
  }
}


Q3CString CRLFtoLF(const Q3CString &s)
{
  Q3CString ret=s.copy();
  ret.replace( "\r\n", "\n" );
  return ret;
}


Q3CString CRLFtoLF(const char *s)
{
  Q3CString ret=s;
  ret.replace( "\r\n", "\n");
  return ret;
}


Q3CString LFtoCRLF(const Q3CString &s)
{
  Q3CString ret=s.copy();
  ret.replace( "\n", "\r\n");
  return ret;
}


void removeQuots(Q3CString &str)
{
  bool inQuote=false;

  for (int i=0; i < (int)str.length(); i++) {
    if (str[i] == '"') {
      str.remove(i,1);
      i--;
      inQuote = !inQuote;
    } else {
      if (inQuote && (str[i] == '\\'))
        str.remove(i,1);
    }
  }
}


void removeQuots(QString &str)
{
  bool inQuote=false;

  for (int i=0; i < (int)str.length(); i++) {
    if (str[i] == '"') {
      str.remove(i,1);
      i--;
      inQuote = !inQuote;
    } else {
      if (inQuote && (str[i] == '\\'))
        str.remove(i,1);
    }
  }
}


void addQuotes(Q3CString &str, bool forceQuotes)
{
  bool needsQuotes=false;
  for (int i=0; i < str.length(); i++) {
    if (strchr("()<>@,.;:[]=\\\"",str[i])!=0)
      needsQuotes = true;
    if (str[i]=='\\' || str[i]=='\"') {
      str.insert(i, '\\');
      i++;
    }
  }

  if (needsQuotes || forceQuotes) {
    str.insert(0,'\"');
    str.append("\"");
  }
}

int DateFormatter::mDaylight = -1;
DateFormatter::DateFormatter(FormatType fType)
  : mFormat( fType ), mCurrentTime( 0 )
{

}

DateFormatter::~DateFormatter()
{/*empty*/}

DateFormatter::FormatType
DateFormatter::getFormat() const
{
  return mFormat;
}

void
DateFormatter::setFormat( FormatType t )
{
  mFormat = t;
}

QString
DateFormatter::dateString( time_t otime , const QString& lang ,
		       bool shortFormat, bool includeSecs ) const
{
  switch ( mFormat ) {
  case Fancy:
    return fancy( otime );
    break;
  case Localized:
    return localized( otime, shortFormat, includeSecs, lang );
    break;
  case CTime:
    return cTime( otime );
    break;
  case Iso:
    return isoDate( otime );
    break;
  case Custom:
    return custom( otime );
    break;
  }
  return QString::null;
}

QString
DateFormatter::dateString(const QDateTime& dtime, const QString& lang,
		       bool shortFormat, bool includeSecs ) const
{
  return DateFormatter::dateString( qdateToTimeT(dtime), lang, shortFormat, includeSecs );
}

Q3CString
DateFormatter::rfc2822(time_t otime) const
{
  QDateTime tmp;
  Q3CString  ret;

  tmp.setTime_t(otime);

  ret = tmp.toString("ddd, dd MMM yyyy hh:mm:ss ").latin1();
  ret += zone(otime);

  return ret;
}

QString
DateFormatter::custom(time_t t) const
{
  if ( mCustomFormat.isEmpty() )
    return QString::null;

  int z = mCustomFormat.find("Z");
  QDateTime d;
  QString ret = mCustomFormat;

  d.setTime_t(t);
  if ( z != -1 ) {
    ret.replace(z,1,zone(t));
  }

  ret = d.toString(ret);

  return ret;
}

void
DateFormatter::setCustomFormat(const QString& format)
{
  mCustomFormat = format;
  mFormat = Custom;
}

QString
DateFormatter::getCustomFormat() const
{
  return mCustomFormat;
}


Q3CString
DateFormatter::zone(time_t otime) const
{
  Q3CString ret;
#if defined(HAVE_TIMEZONE) || defined(HAVE_TM_GMTOFF)
  struct tm *local = localtime( &otime );
#endif

#if defined(HAVE_TIMEZONE)

  //hmm, could make hours & mins static
  int secs = abs(timezone);
  int neg  = (timezone>0)?1:0;
  int hours = secs/3600;
  int mins  = (secs - hours*3600)/60;

  // adjust to daylight
  if ( local->tm_isdst > 0 ) {
      mDaylight = 1;
      if ( neg )
        --hours;
      else
        ++hours;
  } else
      mDaylight = 0;

  ret.sprintf("%c%.2d%.2d",(neg)?'-':'+', hours, mins);

#elif defined(HAVE_TM_GMTOFF)

  int secs = abs( local->tm_gmtoff );
  int neg  = (local->tm_gmtoff<0)?1:0; //no, I don't know why it's backwards :o
  int hours = secs/3600;
  int mins  = (secs - hours*3600)/60;

  if ( local->tm_isdst > 0 )
      mDaylight = 1;
  else
      mDaylight = 0;

  ret.sprintf("%c%.2d%.2d",(neg)?'-':'+', hours, mins);

#else

  QDateTime d1 = QDateTime::fromString( asctime(gmtime(&otime)) );
  QDateTime d2 = QDateTime::fromString( asctime(localtime(&otime)) );
  int secs = d1.secsTo(d2);
  int neg = (secs<0)?1:0;
  secs = abs(secs);
  int hours = secs/3600;
  int mins  = (secs - hours*3600)/60;
  // daylight should be already taken care of here
  ret.sprintf("%c%.2d%.2d",(neg)?'-':'+', hours, mins);

#endif /* HAVE_TIMEZONE */

  return ret;
}

time_t
DateFormatter::qdateToTimeT(const QDateTime& dt) const
{
  QDateTime epoch( QDate(1970, 1,1), QTime(00,00,00) );
  time_t otime;
  time( &otime );

  QDateTime d1 = QDateTime::fromString( asctime(gmtime(&otime)) );
  QDateTime d2 = QDateTime::fromString( asctime(localtime(&otime)) );
  time_t drf = epoch.secsTo( dt ) - d1.secsTo( d2 );

  return drf;
}

QString
DateFormatter::fancy(time_t otime) const
{
  KLocale *locale = KGlobal::locale();

  if ( otime <= 0 )
    return i18n( "unknown" );

  if ( !mCurrentTime ) {
    time( &mCurrentTime );
    mDate.setTime_t( mCurrentTime );
  }

  QDateTime old;
  old.setTime_t( otime );

  // not more than an hour in the future
  if ( mCurrentTime + 60 * 60 >= otime ) {
    time_t diff = mCurrentTime - otime;

    if ( diff < 24 * 60 * 60 ) {
      if ( old.date().year() == mDate.date().year() &&
	   old.date().dayOfYear() == mDate.date().dayOfYear() )
	return i18n( "Today %1" ).arg( locale->
				       formatTime( old.time(), true ) );
    }
    if ( diff < 2 * 24 * 60 * 60 ) {
      QDateTime yesterday( mDate.addDays( -1 ) );
      if ( old.date().year() == yesterday.date().year() &&
	   old.date().dayOfYear() == yesterday.date().dayOfYear() )
	return i18n( "Yesterday %1" ).arg( locale->
					   formatTime( old.time(), true) );
    }
    for ( int i = 3; i < 7; i++ )
      if ( diff < i * 24 * 60 * 60 ) {
	QDateTime weekday( mDate.addDays( -i + 1 ) );
	if ( old.date().year() == weekday.date().year() &&
	     old.date().dayOfYear() == weekday.date().dayOfYear() )
	  return i18n( "1. weekday, 2. time", "%1 %2" ).
	    arg( locale->calendar()->weekDayName( old.date() ) ).
	    arg( locale->formatTime( old.time(), true) );
      }
  }

  return locale->formatDateTime( old );

}

QString
DateFormatter::localized(time_t otime, bool shortFormat, bool includeSecs,
			 const QString& localeLanguage ) const
{
  QDateTime tmp;
  QString ret;
  KLocale *locale = KGlobal::locale();

  tmp.setTime_t( otime );


  if ( !localeLanguage.isEmpty() ) {
    locale=new KLocale(localeLanguage);
    locale->setLanguage(localeLanguage);
    locale->setCountry(localeLanguage);
    ret = locale->formatDateTime( tmp, shortFormat, includeSecs );
    delete locale;
  } else {
    ret = locale->formatDateTime( tmp, shortFormat, includeSecs );
  }

  return ret;
}

QString
DateFormatter::cTime(time_t otime) const
{
  return QString::fromLatin1( ctime(  &otime ) ).stripWhiteSpace() ;
}

QString
DateFormatter::isoDate(time_t otime) const
{
  char cstr[64];
  strftime( cstr, 63, "%Y-%m-%d %H:%M:%S", localtime(&otime) );
  return QString( cstr );
}


void
DateFormatter::reset()
{
  mCurrentTime = 0;
}

QString
DateFormatter::formatDate(DateFormatter::FormatType t, time_t otime,
			  const QString& data, bool shortFormat, bool includeSecs )
{
  DateFormatter f( t );
  if ( t == DateFormatter::Custom ) {
    f.setCustomFormat( data );
  }
  return f.dateString( otime, data, shortFormat, includeSecs );
}

QString
DateFormatter::formatCurrentDate( DateFormatter::FormatType t, const QString& data,
				  bool shortFormat, bool includeSecs )
{
  DateFormatter f( t );
  if ( t == DateFormatter::Custom ) {
    f.setCustomFormat( data );
  }
  return f.dateString( time(0), data, shortFormat, includeSecs );
}

Q3CString
DateFormatter::rfc2822FormatDate( time_t t )
{
  DateFormatter f;
  return f.rfc2822( t );
}

bool
DateFormatter::isDaylight()
{
  if ( mDaylight == -1 ) {
    time_t ntime = time( 0 );
    struct tm *local = localtime( &ntime );
    if ( local->tm_isdst > 0 ) {
      mDaylight = 1;
      return true;
    } else {
      mDaylight = 0;
      return false;
    }
  } else if ( mDaylight != 0 )
    return true;
  else
    return false;
}

} // namespace KMime
