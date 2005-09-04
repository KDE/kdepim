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

#include <q3ptrlist.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3CString>

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


  Q3CString outputParameter (Q3Dict < QString > *);

  int parsePart (mimeIO &, const QString&);
  int parseBody (mimeIO &, Q3CString &, const QString&, bool mbox = false);

  // parse a header. returns true if it had a leading 'From ' line
  bool parseHeader (mimeIO &);

  QString getDispositionParm (const Q3CString&);
  void setDispositionParm (const Q3CString&, const QString&);
  Q3DictIterator < QString > getDispositionIterator ();

  QString getTypeParm (const Q3CString&);
  void setTypeParm (const Q3CString&, const QString&);
  Q3DictIterator < QString > getTypeIterator ();

  // recursively serialize all important contents to the QDataStream
  void serialize(QDataStream& stream);

  const Q3CString& getType ()
  {
    return contentType;
  }
  void setType (const Q3CString & _str)
  {
    contentType = _str;
  }

  const Q3CString& getDescription ()
  {
    return _contentDescription;
  }
  void setDescription (const Q3CString & _str)
  {
    _contentDescription = _str;
  }

  Q3CString getDisposition ()
  {
    return _contentDisposition;
  }
  void setDisposition (const Q3CString & _str)
  {
    _contentDisposition = _str;
  }

  Q3CString getEncoding ()
  {
    return contentEncoding;
  }
  void setEncoding (const Q3CString & _str)
  {
    contentEncoding = _str;
  }

  Q3CString getMD5 ()
  {
    return contentMD5;
  }
  void setMD5 (const Q3CString & _str)
  {
    contentMD5 = _str;
  }

  Q3CString getID ()
  {
    return contentID;
  }
  void setID (const Q3CString & _str)
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

  const QString & getPartSpecifier ()
  {
    return partSpecifier;
  }
  void setPartSpecifier (const QString & _str)
  {
    partSpecifier = _str;
  }

  Q3PtrListIterator < mimeHdrLine > getOriginalIterator ();
  Q3PtrListIterator < mimeHdrLine > getAdditionalIterator ();
  void setContent (const Q3CString &aContent)
  {
    mimeContent = aContent;
  }
  Q3CString getContent ()
  {
    return mimeContent;
  }

  Q3CString getBody ()
  {
    return preMultipartBody + postMultipartBody;
  }
  Q3CString getPreBody ()
  {
    return preMultipartBody;
  }
  void setPreBody (Q3CString & inBody)
  {
    preMultipartBody = inBody;
  }

  Q3CString getPostBody ()
  {
    return postMultipartBody;
  }
  void setPostBody (Q3CString & inBody)
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
  Q3PtrListIterator < mimeHeader > getNestedIterator ()
  {
    return Q3PtrListIterator < mimeHeader > (nestedParts);
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
  mimeHeader *bodyPart (const QString &);

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
  void write (const QString &)
  {
  }
  QString typeStr ()
  {
    return QString (contentType.left (contentType.find ('/')));
  }
  void setTypeStr (const QString & _str)
  {
    contentType = Q3CString (_str.latin1 ()) + "/" + subtypeStr ().latin1 ();
  }
  QString subtypeStr ()
  {
    return QString (contentType.
                    right (contentType.length () - contentType.find ('/') -
                           1));
  }
  void setSubtypeStr (const QString & _str)
  {
    contentType = Q3CString (typeStr ().latin1 ()) + "/" + _str.latin1 ();
  }
  QString cteStr ()
  {
    return QString (getEncoding ());
  }
  void setCteStr (const QString & _str)
  {
    setEncoding (_str.latin1 ());
  }
  QString contentDisposition ()
  {
    return QString (_contentDisposition);
  }
  QString body ()
  {
    return QString (postMultipartBody);
  }
  QString charset ()
  {
    return getTypeParm ("charset");
  }
  QString bodyDecoded ();
  void setBodyEncoded (const QByteArray &);
  void setBodyEncodedBinary (const QByteArray &);
  QByteArray bodyDecodedBinary ();
  QString name ()
  {
    return QString (getTypeParm ("name"));
  }
  void setName (const QString & _str)
  {
    setTypeParm ("name", _str);
  }
  QString fileName ()
  {
    return QString (getDispositionParm ("filename"));
  }
  QString contentDescription ()
  {
    return QString (rfcDecoder::decodeRFC2047String (_contentDescription));
  }
  void setContentDescription (const QString & _str)
  {
    _contentDescription = rfcDecoder::encodeRFC2047String (_str).latin1 ();
  }
  QString msgIdMD5 ()
  {
    return QString (contentMD5);
  }
  QString iconName ();
  QString magicSetType (bool aAutoDecode = true);
  QString headerAsString ();
  ulong size ()
  {
    return 0;
  }
  void fromString (const QByteArray &)
  {;
  }
  void setContentDisposition (const QString & _str)
  {
    setDisposition (_str.latin1 ());
  }
#endif

protected:
  static void addParameter (const Q3CString&, Q3Dict < QString > *);
  static QString getParameter (const Q3CString&, Q3Dict < QString > *);
  static void setParameter (const Q3CString&, const QString&, Q3Dict < QString > *);

  Q3PtrList < mimeHdrLine > originalHdrLines;

private:
  Q3PtrList < mimeHdrLine > additionalHdrLines;
  Q3Dict < QString > typeList;
  Q3Dict < QString > dispositionList;
  Q3CString contentType;
  Q3CString _contentDisposition;
  Q3CString contentEncoding;
  Q3CString _contentDescription;
  Q3CString contentID;
  Q3CString contentMD5;
  unsigned long contentLength;
  Q3CString mimeContent;
  Q3CString preMultipartBody;
  Q3CString postMultipartBody;
  mimeHeader *nestedMessage;
  Q3PtrList < mimeHeader > nestedParts;
  QString partSpecifier;

};

#endif
