/**********************************************************************
 *
 *   rfcdecoder.cc  - handler for various rfc/mime encodings
 *   Copyright (C) 2000 s.carstens@gmx.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *   Send comments and bug fixes to s.carstens@gmx.de
 *
 *********************************************************************/
#include "rfcdecoder.h"

#include <ctype.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include <QTextCodec>
#include <QBuffer>
#include <QRegExp>
//Added by qt3to4:
#include <Q3CString>
#include <QLatin1Char>
#include <kcodecs.h>

// This part taken from rfc 2192 IMAP URL Scheme. C. Newman. September 1997.
// adapted to QT-Toolkit by Sven Carstens <s.carstens@gmx.de> 2000

static unsigned char base64chars[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";
#define UNDEFINED 64
#define MAXLINE  76

/* UTF16 definitions */
#define UTF16MASK       0x03FFUL
#define UTF16SHIFT      10
#define UTF16BASE       0x10000UL
#define UTF16HIGHSTART  0xD800UL
#define UTF16HIGHEND    0xDBFFUL
#define UTF16LOSTART    0xDC00UL
#define UTF16LOEND      0xDFFFUL

/* Convert an IMAP mailbox to a Unicode path
 */
QString rfcDecoder::fromIMAP (const QString & inSrc)
{
  unsigned char c, i, bitcount;
  unsigned long ucs4, utf16, bitbuf;
  unsigned char base64[256], utf8[6];
  unsigned int srcPtr = 0;
  QByteArray dst;
  QByteArray src = inSrc.toAscii ();
  uint srcLen = inSrc.length();

  /* initialize modified base64 decoding table */
  memset (base64, UNDEFINED, sizeof (base64));
  for (i = 0; i < sizeof (base64chars); ++i)
  {
    base64[(int)base64chars[i]] = i;
  }

  /* loop until end of string */
  while (srcPtr < srcLen)
  {
    c = src[srcPtr++];
    /* deal with literal characters and &- */
    if (c != '&' || src[srcPtr] == '-')
    {
      /* encode literally */
      dst += c;
      /* skip over the '-' if this is an &- sequence */
      if (c == '&')
        srcPtr++;
    }
    else
    {
      /* convert modified UTF-7 -> UTF-16 -> UCS-4 -> UTF-8 -> HEX */
      bitbuf = 0;
      bitcount = 0;
      ucs4 = 0;
      while ((c = base64[(unsigned char) src[srcPtr]]) != UNDEFINED)
      {
        ++srcPtr;
        bitbuf = (bitbuf << 6) | c;
        bitcount += 6;
        /* enough bits for a UTF-16 character? */
        if (bitcount >= 16)
        {
          bitcount -= 16;
          utf16 = (bitcount ? bitbuf >> bitcount : bitbuf) & 0xffff;
          /* convert UTF16 to UCS4 */
          if (utf16 >= UTF16HIGHSTART && utf16 <= UTF16HIGHEND)
          {
            ucs4 = (utf16 - UTF16HIGHSTART) << UTF16SHIFT;
            continue;
          }
          else if (utf16 >= UTF16LOSTART && utf16 <= UTF16LOEND)
          {
            ucs4 += utf16 - UTF16LOSTART + UTF16BASE;
          }
          else
          {
            ucs4 = utf16;
          }
          /* convert UTF-16 range of UCS4 to UTF-8 */
          if (ucs4 <= 0x7fUL)
          {
            utf8[0] = ucs4;
            i = 1;
          }
          else if (ucs4 <= 0x7ffUL)
          {
            utf8[0] = 0xc0 | (ucs4 >> 6);
            utf8[1] = 0x80 | (ucs4 & 0x3f);
            i = 2;
          }
          else if (ucs4 <= 0xffffUL)
          {
            utf8[0] = 0xe0 | (ucs4 >> 12);
            utf8[1] = 0x80 | ((ucs4 >> 6) & 0x3f);
            utf8[2] = 0x80 | (ucs4 & 0x3f);
            i = 3;
          }
          else
          {
            utf8[0] = 0xf0 | (ucs4 >> 18);
            utf8[1] = 0x80 | ((ucs4 >> 12) & 0x3f);
            utf8[2] = 0x80 | ((ucs4 >> 6) & 0x3f);
            utf8[3] = 0x80 | (ucs4 & 0x3f);
            i = 4;
          }
          /* copy it */
          for (c = 0; c < i; ++c)
          {
            dst += utf8[c];
          }
        }
      }
      /* skip over trailing '-' in modified UTF-7 encoding */
      if (src[srcPtr] == '-')
        ++srcPtr;
    }
  }
  return QString::fromUtf8 (dst.data ());
}

/* replace " with \" and \ with \\ " and \ characters */
QString rfcDecoder::quoteIMAP(const QString &src)
{
  uint len = src.length();
  QString result;
  result.reserve(2 * len);
  for (unsigned int i = 0; i < len; i++)
  {
    if (src[i] == '"' || src[i] == '\\')
      result += '\\';
    result += src[i];
  }
  //result.squeeze(); - unnecessary and slow
  return result;
}

/* Convert Unicode path to modified UTF-7 IMAP mailbox
 */
QString rfcDecoder::toIMAP (const QString & inSrc)
{
  unsigned int utf8pos, utf8total, c, utf7mode, bitstogo, utf16flag;
  unsigned int ucs4, bitbuf;
  Q3CString src = inSrc.toUtf8 ();
  QString dst;

  int srcPtr = 0;
  utf7mode = 0;
  utf8total = 0;
  bitstogo = 0;
  utf8pos = 0;
  bitbuf = 0;
  ucs4 = 0;
  while (srcPtr < src.length ())
  {
    c = (unsigned char) src[srcPtr++];
    /* normal character? */
    if (c >= ' ' && c <= '~')
    {
      /* switch out of UTF-7 mode */
      if (utf7mode)
      {
        if (bitstogo)
        {
          dst += base64chars[(bitbuf << (6 - bitstogo)) & 0x3F];
          bitstogo = 0;
        }
        dst += '-';
        utf7mode = 0;
      }
      dst += c;
      /* encode '&' as '&-' */
      if (c == '&')
      {
        dst += '-';
      }
      continue;
    }
    /* switch to UTF-7 mode */
    if (!utf7mode)
    {
      dst += '&';
      utf7mode = 1;
    }
    /* Encode US-ASCII characters as themselves */
    if (c < 0x80)
    {
      ucs4 = c;
      utf8total = 1;
    }
    else if (utf8total)
    {
      /* save UTF8 bits into UCS4 */
      ucs4 = (ucs4 << 6) | (c & 0x3FUL);
      if (++utf8pos < utf8total)
      {
        continue;
      }
    }
    else
    {
      utf8pos = 1;
      if (c < 0xE0)
      {
        utf8total = 2;
        ucs4 = c & 0x1F;
      }
      else if (c < 0xF0)
      {
        utf8total = 3;
        ucs4 = c & 0x0F;
      }
      else
      {
        /* NOTE: can't convert UTF8 sequences longer than 4 */
        utf8total = 4;
        ucs4 = c & 0x03;
      }
      continue;
    }
    /* loop to split ucs4 into two utf16 chars if necessary */
    utf8total = 0;
    do
    {
      if (ucs4 >= UTF16BASE)
      {
        ucs4 -= UTF16BASE;
        bitbuf = (bitbuf << 16) | ((ucs4 >> UTF16SHIFT) + UTF16HIGHSTART);
        ucs4 = (ucs4 & UTF16MASK) + UTF16LOSTART;
        utf16flag = 1;
      }
      else
      {
        bitbuf = (bitbuf << 16) | ucs4;
        utf16flag = 0;
      }
      bitstogo += 16;
      /* spew out base64 */
      while (bitstogo >= 6)
      {
        bitstogo -= 6;
        dst += base64chars[(bitstogo ? (bitbuf >> bitstogo) : bitbuf) & 0x3F];
      }
    }
    while (utf16flag);
  }
  /* if in UTF-7 mode, finish in ASCII */
  if (utf7mode)
  {
    if (bitstogo)
    {
      dst += base64chars[(bitbuf << (6 - bitstogo)) & 0x3F];
    }
    dst += '-';
  }
  return quoteIMAP(dst);
}

//-----------------------------------------------------------------------------
QTextCodec *
rfcDecoder::codecForName (const QString & _str)
{
  if (_str.isEmpty ())
    return NULL;
  return QTextCodec::codecForName (_str.toLower ().
                                   replace ("windows", "cp").toLatin1 ());
}

//-----------------------------------------------------------------------------
const QString
rfcDecoder::decodeRFC2047String (const QString & _str)
{
  QString throw_away;

  return decodeRFC2047String (_str, throw_away);
}

//-----------------------------------------------------------------------------
const QString
rfcDecoder::decodeRFC2047String (const QString & _str, QString & charset)
{
  QString throw_away;

  return decodeRFC2047String (_str, charset, throw_away);
}

//-----------------------------------------------------------------------------
const QString
rfcDecoder::decodeRFC2047String (const QString & _str, QString & charset,
                                 QString & language)
{
  //do we have a rfc string
  if (!_str.contains("=?") )
    return _str;

  // FIXME get rid of the conversion?
  QByteArray aStr = _str.toAscii ();  // QString.length() means Unicode chars
  QByteArray result;
  char *pos, *beg, *end, *mid = NULL;
  Q3CString str;
  char encoding = 0, ch;
  bool valid;
  const int maxLen = 200;
  int i;

//  result.truncate(aStr.length());
  for (pos = aStr.data (); *pos; pos++)
  {
    if (pos[0] != '=' || pos[1] != '?')
    {
      result += *pos;
      continue;
    }
    beg = pos + 2;
    end = beg;
    valid = true;
    // parse charset name
    for (i = 2, pos += 2;
         i < maxLen && (*pos != '?' && (ispunct (*pos) || isalnum (*pos)));
         i++)
      pos++;
    if (*pos != '?' || i < 4 || i >= maxLen)
      valid = false;
    else
    {
      charset = Q3CString (beg, i - 1);  // -2 + 1 for the zero
      int pt = charset.lastIndexOf('*');
      if (pt != -1)
      {
        // save language for later usage
        language = charset.right (charset.length () - pt - 1);

        // tie off language as defined in rfc2047
        charset.truncate(pt);
      }
      // get encoding and check delimiting question marks
      encoding = toupper (pos[1]);
      if (pos[2] != '?'
          || (encoding != 'Q' && encoding != 'B' && encoding != 'q'
              && encoding != 'b'))
        valid = false;
      pos += 3;
      i += 3;
//    kDebug(7116) << "rfcDecoder::decodeRFC2047String - charset " << charset << " - language " << language << " - '" << pos << "'" << endl;
    }
    if (valid)
    {
      mid = pos;
      // search for end of encoded part
      while (i < maxLen && *pos && !(*pos == '?' && *(pos + 1) == '='))
      {
        i++;
        pos++;
      }
      end = pos + 2;            //end now points to the first char after the encoded string
      if (i >= maxLen || !*pos)
        valid = false;
    }
    if (valid)
    {
      ch = *pos;
      *pos = '\0';
      str = Q3CString (mid).left ((int) (mid - pos - 1));
      if (encoding == 'Q')
      {
        // decode quoted printable text
        for (i = str.length () - 1; i >= 0; i--)
          if (str[i] == '_')
            str[i] = ' ';
//    kDebug(7116) << "rfcDecoder::decodeRFC2047String - before QP '" << str << "'" << endl;

        str = KCodecs::quotedPrintableDecode(str);
//    kDebug(7116) << "rfcDecoder::decodeRFC2047String - after QP '" << str << "'" << endl;
      }
      else
      {
        // decode base64 text
        str = KCodecs::base64Decode(str);
      }
      *pos = ch;
      int len = str.length();
      for (i = 0; i < len; i++)
        result += str[i];

      pos = end - 1;
    }
    else
    {
//    kDebug(7116) << "rfcDecoder::decodeRFC2047String - invalid" << endl;
      //result += "=?";
      //pos = beg -1; // because pos gets increased shortly afterwards
      pos = beg - 2;
      result += *pos++;
      result += *pos;
    }
  }
  if (!charset.isEmpty ())
  {
    QTextCodec *aCodec = codecForName (charset.toAscii ());
    if (aCodec)
    {
//    kDebug(7116) << "Codec is " << aCodec->name() << endl;
      return aCodec->toUnicode (result);
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
const char especials[17] = "()<>@,;:\"/[]?.= ";

const QString
rfcDecoder::encodeRFC2047String (const QString & _str)
{
  if (_str.isEmpty ())
    return _str;
  const signed char *latin = reinterpret_cast<const signed char *>(_str.toLatin1().data()), *l, *start, *stop;
  char hexcode;
  int numQuotes, i;
  int rptr = 0;
  // My stats show this number results in 12 resize() out of 73,000
  int resultLen = 3 * _str.length() / 2;
  QByteArray result(resultLen, '\0');

  while (*latin)
  {
    l = latin;
    start = latin;
    while (*l)
    {
      if (*l == 32)
        start = l + 1;
      if (*l < 0)
        break;
      l++;
    }
    if (*l)
    {
      numQuotes = 1;
      while (*l)
      {
        /* The encoded word must be limited to 75 character */
        for (i = 0; i < 16; i++)
          if (*l == especials[i])
            numQuotes++;
        if (*l < 0)
          numQuotes++;
        /* Stop after 58 = 75 - 17 characters or at "<user@host..." */
        if (l - start + 2 * numQuotes >= 58 || *l == 60)
          break;
        l++;
      }
      if (*l)
      {
        stop = l - 1;
        while (stop >= start && *stop != 32)
          stop--;
        if (stop <= start)
          stop = l;
      }
      else
        stop = l;
      if (resultLen - rptr - 1 <= start -  latin + 1 + 16 /* =?iso-88... */) {
        resultLen += (start - latin + 1) * 2 + 20; // more space
	result.resize(resultLen);
      }
      while (latin < start)
      {
        result[rptr++] = *latin;
        latin++;
      }
      result.replace( rptr, 15, "=?iso-8859-1?q?" );
      rptr += 15;
      if (resultLen - rptr - 1 <= 3*(stop - latin + 1)) {
        resultLen += (stop - latin + 1) * 4 + 20; // more space
	result.resize(resultLen);
      }
      while (latin < stop) // can add up to 3 chars/iteration
      {
        numQuotes = 0;
        for (i = 0; i < 16; i++)
          if (*latin == especials[i])
            numQuotes = 1;
        if (*latin < 0)
          numQuotes = 1;
        if (numQuotes)
        {
          result[rptr++] = '=';
          hexcode = ((*latin & 0xF0) >> 4) + 48;
          if (hexcode >= 58)
            hexcode += 7;
          result[rptr++] = hexcode;
          hexcode = (*latin & 0x0F) + 48;
          if (hexcode >= 58)
            hexcode += 7;
          result[rptr++] = hexcode;
        }
        else
        {
          result[rptr++] = *latin;
        }
        latin++;
      }
      result[rptr++] = '?';
      result[rptr++] = '=';
    }
    else
    {
      while (*latin)
      {
        if (rptr == resultLen - 1) {
          resultLen += 30;
          result.resize(resultLen);
        }
        result[rptr++] = *latin;
        latin++;
      }
    }
  }
  result[rptr] = 0;
  //free (latinStart);
  return result;
}


//-----------------------------------------------------------------------------
const QString
rfcDecoder::encodeRFC2231String (const QString & _str)
{
  if (_str.isEmpty ())
    return _str;
  signed char *latin = (signed char *) calloc (1, _str.length () + 1);
  char *latin_us = (char *) latin;
  strcpy (latin_us, _str.toLatin1 ());
  signed char *l = latin;
  char hexcode;
  int i;
  bool quote;
  while (*l)
  {
    if (*l < 0)
      break;
    l++;
  }
  if (!*l) {
    free(latin);
    return _str;
  }
  Q3CString result;
  l = latin;
  while (*l)
  {
    quote = *l < 0;
    for (i = 0; i < 16; i++)
      if (*l == especials[i])
        quote = true;
    if (quote)
    {
      result += "%";
      hexcode = ((*l & 0xF0) >> 4) + 48;
      if (hexcode >= 58)
        hexcode += 7;
      result += hexcode;
      hexcode = (*l & 0x0F) + 48;
      if (hexcode >= 58)
        hexcode += 7;
      result += hexcode;
    }
    else
    {
      result += *l;
    }
    l++;
  }
  free (latin);
  return result;
}


//-----------------------------------------------------------------------------
const QString
rfcDecoder::decodeRFC2231String (const QString & _str)
{
  int p = _str.indexOf ('\'');

  //see if it is an rfc string
  if (p < 0)
    return _str;

  int l = _str.lastIndexOf ('\'');

  //second is language
  if (p >= l)
    return _str;

  //first is charset or empty
  QString charset = _str.left (p);
  QString st = _str.mid (l + 1);
  QString language = _str.mid (p + 1, l - p - 1);

  //kDebug(7116) << "Charset: " << charset << " Language: " << language << endl;

  char ch, ch2;
  p = 0;
  while (p < (int) st.length ())
  {
    if (st.at (p) == 37)
    {
      ch = st.at (p + 1).toLatin1 () - 48;
      if (ch > 16)
        ch -= 7;
      ch2 = st.at (p + 2).toLatin1 () - 48;
      if (ch2 > 16)
        ch2 -= 7;
      st.replace( p, 1, ch * 16 + ch2 );
      st.remove ( p + 1, 2 );
    }
    p++;
  }
  return st;
}
