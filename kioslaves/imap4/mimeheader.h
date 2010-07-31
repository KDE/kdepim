/***************************************************************************
                          mimeheader.h  -  description
                             -------------------
    begin                : Fri Oct 20 2000
    copyright            : (C) 2000 by Sven Carstens
    email                : s.carstens@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIMEHEADER_H
#define MIMEHEADER_H

#include <tqptrlist.h>
#include <tqdict.h>

#include "mimehdrline.h"
#include "mimeio.h"
#include "rfcdecoder.h"

/**
  *@author Sven Carstens
  */

class mimeHeader
{
public:
  mimeHeader ();
  virtual ~ mimeHeader ();

  virtual void addHdrLine (mimeHdrLine *);
  virtual void outputHeader (mimeIO &);
  virtual void outputPart (mimeIO &);


  TQCString outputParameter (TQDict < TQString > *);

  int parsePart (mimeIO &, const TQString&);
  int parseBody (mimeIO &, TQCString &, const TQString&, bool mbox = false);

  // parse a header. returns true if it had a leading 'From ' line
  bool parseHeader (mimeIO &);

  TQString getDispositionParm (const TQCString&);
  void setDispositionParm (const TQCString&, const TQString&);
  TQDictIterator < TQString > getDispositionIterator ();

  TQString getTypeParm (const TQCString&);
  void setTypeParm (const TQCString&, const TQString&);
  TQDictIterator < TQString > getTypeIterator ();

  // recursively serialize all important contents to the QDataStream
  void serialize(TQDataStream& stream);

  const TQCString& getType ()
  {
    return contentType;
  }
  void setType (const TQCString & _str)
  {
    contentType = _str;
  }

  const TQCString& getDescription ()
  {
    return _contentDescription;
  }
  void setDescription (const TQCString & _str)
  {
    _contentDescription = _str;
  }

  TQCString getDisposition ()
  {
    return _contentDisposition;
  }
  void setDisposition (const TQCString & _str)
  {
    _contentDisposition = _str;
  }

  TQCString getEncoding ()
  {
    return contentEncoding;
  }
  void setEncoding (const TQCString & _str)
  {
    contentEncoding = _str;
  }

  TQCString getMD5 ()
  {
    return contentMD5;
  }
  void setMD5 (const TQCString & _str)
  {
    contentMD5 = _str;
  }

  TQCString getID ()
  {
    return contentID;
  }
  void setID (const TQCString & _str)
  {
    contentID = _str;
  }

  unsigned long getLength ()
  {
    return contentLength;
  }
  void setLength (unsigned long _len)
  {
    contentLength = _len;
  }

  const TQString & getPartSpecifier ()
  {
    return partSpecifier;
  }
  void setPartSpecifier (const TQString & _str)
  {
    partSpecifier = _str;
  }

  TQPtrListIterator < mimeHdrLine > getOriginalIterator ();
  TQPtrListIterator < mimeHdrLine > getAdditionalIterator ();
  void setContent (const TQCString &aContent)
  {
    mimeContent = aContent;
  }
  TQCString getContent ()
  {
    return mimeContent;
  }

  TQCString getBody ()
  {
    return preMultipartBody + postMultipartBody;
  }
  TQCString getPreBody ()
  {
    return preMultipartBody;
  }
  void setPreBody (TQCString & inBody)
  {
    preMultipartBody = inBody;
  }

  TQCString getPostBody ()
  {
    return postMultipartBody;
  }
  void setPostBody (TQCString & inBody)
  {
    postMultipartBody = inBody;
    contentLength = inBody.length ();
  }

  mimeHeader *getNestedMessage ()
  {
    return nestedMessage;
  }
  void setNestedMessage (mimeHeader * inPart, bool destroy = true)
  {
    if (nestedMessage && destroy)
      delete nestedMessage;
    nestedMessage = inPart;
  }

//  mimeHeader *getNestedPart() { return nestedPart; };
  void addNestedPart (mimeHeader * inPart)
  {
    nestedParts.append (inPart);
  }
  TQPtrListIterator < mimeHeader > getNestedIterator ()
  {
    return TQPtrListIterator < mimeHeader > (nestedParts);
  }

  // clears all parts and deletes them from memory
  void clearNestedParts ()
  {
    nestedParts.clear ();
  }

  // clear all parameters to content-type
  void clearTypeParameters ()
  {
    typeList.clear ();
  }

  // clear all parameters to content-disposition
  void clearDispositionParameters ()
  {
    dispositionList.clear ();
  }

  // return the specified body part or NULL
  mimeHeader *bodyPart (const TQString &);

#ifdef KMAIL_COMPATIBLE
  ulong msgSize ()
  {
    return contentLength;
  }
  uint numBodyParts ()
  {
    return nestedParts.count ();
  }
  mimeHeader *bodyPart (int which, mimeHeader ** ret = NULL)
  {
    if (ret)
      (*ret) = nestedParts.at (which);
    return nestedParts.at (which);
  }
  void write (const TQString &)
  {
  }
  TQString typeStr ()
  {
    return TQString (contentType.left (contentType.find ('/')));
  }
  void setTypeStr (const TQString & _str)
  {
    contentType = TQCString (_str.latin1 ()) + "/" + subtypeStr ().latin1 ();
  }
  TQString subtypeStr ()
  {
    return TQString (contentType.
                    right (contentType.length () - contentType.find ('/') -
                           1));
  }
  void setSubtypeStr (const TQString & _str)
  {
    contentType = TQCString (typeStr ().latin1 ()) + "/" + _str.latin1 ();
  }
  TQString cteStr ()
  {
    return TQString (getEncoding ());
  }
  void setCteStr (const TQString & _str)
  {
    setEncoding (_str.latin1 ());
  }
  TQString contentDisposition ()
  {
    return TQString (_contentDisposition);
  }
  TQString body ()
  {
    return TQString (postMultipartBody);
  }
  TQString charset ()
  {
    return getTypeParm ("charset");
  }
  TQString bodyDecoded ();
  void setBodyEncoded (const TQByteArray &);
  void setBodyEncodedBinary (const TQByteArray &);
  TQByteArray bodyDecodedBinary ();
  TQString name ()
  {
    return TQString (getTypeParm ("name"));
  }
  void setName (const TQString & _str)
  {
    setTypeParm ("name", _str);
  }
  TQString fileName ()
  {
    return TQString (getDispositionParm ("filename"));
  }
  TQString contentDescription ()
  {
    return TQString (rfcDecoder::decodeRFC2047String (_contentDescription));
  }
  void setContentDescription (const TQString & _str)
  {
    _contentDescription = rfcDecoder::encodeRFC2047String (_str).latin1 ();
  }
  TQString msgIdMD5 ()
  {
    return TQString (contentMD5);
  }
  TQString iconName ();
  TQString magicSetType (bool aAutoDecode = true);
  TQString headerAsString ();
  ulong size ()
  {
    return 0;
  }
  void fromString (const TQByteArray &)
  {;
  }
  void setContentDisposition (const TQString & _str)
  {
    setDisposition (_str.latin1 ());
  }
#endif

protected:
  static void addParameter (const TQCString&, TQDict < TQString > *);
  static TQString getParameter (const TQCString&, TQDict < TQString > *);
  static void setParameter (const TQCString&, const TQString&, TQDict < TQString > *);

  TQPtrList < mimeHdrLine > originalHdrLines;

private:
  TQPtrList < mimeHdrLine > additionalHdrLines;
  TQDict < TQString > typeList;
  TQDict < TQString > dispositionList;
  TQCString contentType;
  TQCString _contentDisposition;
  TQCString contentEncoding;
  TQCString _contentDescription;
  TQCString contentID;
  TQCString contentMD5;
  unsigned long contentLength;
  TQCString mimeContent;
  TQCString preMultipartBody;
  TQCString postMultipartBody;
  mimeHeader *nestedMessage;
  TQPtrList < mimeHeader > nestedParts;
  TQString partSpecifier;

};

#endif
