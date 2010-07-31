/* -*- mode: C++ -*-
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef kmmsgpart_h
#define kmmsgpart_h

#include <kiconloader.h>

#include <tqstring.h>
#include <tqcstring.h>
#include <tqdict.h>

template <typename T>
class QValueList;
class QTextCodec;
class DwString;

class KMMessagePart
{
public:
  KMMessagePart();
  KMMessagePart( TQDataStream & stream );
  virtual ~KMMessagePart();

  /** Reset to text/plain with 7bit cte and clear all other properties. */
  void clear();

  /** Obtains an independant copy (i.e. without explicitely shared data) of the
      data contained in msgPart. */
  void duplicate( const KMMessagePart & msgPart );

  /** Get or set the message body */
  TQCString body(void) const;
  void setBody(const TQCString &aStr);
  DwString dwBody() const;
  void setBody(const DwString &aStr);
  // warning, doesn't detach from 'arr'
  void setBody(const TQByteArray &arr);

  /** Sets this body part's content to @p str. @p str is subject to
      automatic charset and CTE detection.
   **/
  void setBodyFromUnicode( const TQString & str );

  /** Returns the body part decoded to unicode.
   **/
  TQString bodyToUnicode(const TQTextCodec* codec=0) const;

  /** Returns body as decoded string. Assumes that content-transfer-encoding
    contains the correct encoding. This routine is meant for binary data.
    No trailing 0 is appended. */
  TQByteArray bodyDecodedBinary(void) const;

  /** Returns body as decoded string. Assumes that content-transfer-encoding
      contains the correct encoding. This routine is meant for text strings! */
  TQCString bodyDecoded(void) const;

  /** Sets body, encoded in the best fitting
      content-transfer-encoding, which is determined by character
      frequency count.

      @param aBuf       input buffer
      @param allowedCte return: list of allowed cte's
      @param allow8Bit  whether "8bit" is allowed as cte.
      @param willBeSigned whether "7bit"/"8bit" is allowed as cte according to RFC 3156
  */
  void setBodyAndGuessCte(const TQByteArray& aBuf,
				  TQValueList<int>& allowedCte,
				  bool allow8Bit = false,
                                  bool willBeSigned = false);
  /** Same for text */
  void setBodyAndGuessCte(const TQCString& aBuf,
				  TQValueList<int>& allowedCte,
				  bool allow8Bit = false,
                                  bool willBeSigned = false);

  /** Sets body, encoded according to the content-transfer-encoding.
      BEWARE: The entire aStr is used including trailing 0 of text strings!
      This version is faster than setBodyEncoded, no duplication necessary.
    */
  void setBodyEncodedBinary(const TQByteArray& aStr);

  /** Sets body, encoded according to the content-transfer-encoding.
      This one is for text strings, the trailing 0 is not used.

      For speed reasons, prefer setBodyEncodedBinary.
      When possible (the TQCString isn't used afterwards), change setBodyEncoded(myQCString) into:
       setBodyEncodedBinary(byteArrayFromQCStringNoDetach(myQCString));
  */
 void setBodyEncoded(const TQCString& aStr);

  /** Set a full message string as the body of the message part,
      disallowing anything but 7bit or 8bit encoding.
      (RFC 1521 section 7.3)
  */
 void setMessageBody( const TQByteArray & aBuf );

  /** Returns decoded length of body. */
  int decodedSize(void) const;

  /** Get or set the 'Content-Type' header field
   The member functions that involve enumerated types (ints)
   will work only for well-known types or subtypes. */
  TQCString originalContentTypeStr(void) const { return mOriginalContentTypeStr; }
  void setOriginalContentTypeStr( const TQCString& txt )
  {
    mOriginalContentTypeStr = txt;
  }
  TQCString typeStr() const { return mType; }
  void setTypeStr( const TQCString & aStr ) { mType = aStr; }
  int type() const;
  void setType(int aType);
  /** Subtype */
  TQCString subtypeStr() const { return mSubtype; }
  void setSubtypeStr( const TQCString & aStr ) { mSubtype = aStr; }
  int subtype() const;
  void setSubtype(int aSubtype);

  /** Content-Id */
  TQCString contentId() const { return mContentId; }
  void setContentId( const TQCString & aStr ) { mContentId = aStr; }

  /** Set the 'Content-Type' by mime-magic from the contents of the body.
    If autoDecode is TRUE the decoded body will be used for mime type
    determination (this does not change the body itself). */
  void magicSetType(bool autoDecode=TRUE);

  /** Get or set a custom content type parameter, consisting of an attribute
    name and a corresponding value. */
  TQCString parameterAttribute(void) const;
  TQString parameterValue(void) const;
  void setParameter(const TQCString &attribute, const TQString &value);

  TQCString additionalCTypeParamStr(void) const
  {
    return mAdditionalCTypeParamStr;
  }
  void setAdditionalCTypeParamStr( const TQCString &param )
  {
    mAdditionalCTypeParamStr = param;
  }

  /** Tries to find a good icon for the 'Content-Type' by scanning
    the installed mimelnk files. Returns the found icon. If no matching
    icon is found, the one for application/octet-stream is returned. */
  TQString iconName( int size = KIcon::Desktop ) const;

  /** Get or set the 'Content-Transfer-Encoding' header field
    The member functions that involve enumerated types (ints)
    will work only for well-known encodings. */
  TQCString contentTransferEncodingStr(void) const;
  int  contentTransferEncoding(void) const;
  void setContentTransferEncodingStr(const TQCString &aStr);
  void setContentTransferEncoding(int aCte);

  /** Cte is short for ContentTransferEncoding.
      These functions are an alternative to the ones with longer names. */
  TQCString cteStr(void) const { return contentTransferEncodingStr(); }
  int cte(void) const { return contentTransferEncoding(); }
  void setCteStr(const TQCString& aStr) { setContentTransferEncodingStr(aStr); }
  void setCte(int aCte) { setContentTransferEncoding(aCte); }


  /** Get or set the 'Content-Description' header field */
  TQString contentDescription() const;
  TQCString contentDescriptionEncoded() const { return mContentDescription; }
  void setContentDescription(const TQString &aStr);

  /** Get or set the 'Content-Disposition' header field */
  TQCString contentDisposition() const { return mContentDisposition; }
  void setContentDisposition( const TQCString & cd ) { mContentDisposition = cd; }

  /** Get the message part charset.*/
  TQCString charset() const { return mCharset; }

  /** Set the message part charset. */
  void setCharset( const TQCString & c );

  /** Get a TQTextCodec suitable for this message part */
  const TQTextCodec * codec() const;

  /** Get or set name parameter */
  TQString name() const { return mName; }
  void setName( const TQString & name ) { mName = name; }

  /** Returns name of filename part of 'Content-Disposition' header field,
      if present. */
  TQString fileName(void) const;

  /** Returns the part number */
  TQString partSpecifier() const { return mPartSpecifier; }

  /** Sets the part number */
  void setPartSpecifier( const TQString & part ) { mPartSpecifier = part; }

  /** If this part is complete (contains a body) */
  bool isComplete() { return (!mBody.isNull()); }

  /** Returns the parent part */
  KMMessagePart* parent() { return mParent; }

  /** Set the parent of this part */
  void setParent( KMMessagePart* part ) { mParent = part; }

  /** Returns true if the headers should be loaded */
  bool loadHeaders() { return mLoadHeaders; }

  /** Set to true if the headers should be loaded */
  void setLoadHeaders( bool load ) { mLoadHeaders = load; }

  /** Returns true if the part itself (as returned by kioslave) should be loaded */
  bool loadPart() { return mLoadPart; }

  /** Set to true if the part itself should be loaded */
  void setLoadPart( bool load ) { mLoadPart = load; }

protected:
  TQCString mOriginalContentTypeStr;
  TQCString mType;
  TQCString mSubtype;
  TQCString mCte;
  TQCString mContentDescription;
  TQCString mContentDisposition;
  TQCString mContentId;
  TQByteArray mBody;
  TQCString mAdditionalCTypeParamStr;
  TQString mName;
  TQCString mParameterAttribute;
  TQString mParameterValue;
  TQCString mCharset;
  TQString mPartSpecifier;
  mutable int mBodyDecodedSize;
  KMMessagePart* mParent;
  bool mLoadHeaders;
  bool mLoadPart;
};


#endif /*kmmsgpart_h*/
