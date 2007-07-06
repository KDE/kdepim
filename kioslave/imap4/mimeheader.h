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
#include <QByteArray>

#include "mimehdrline.h"
#include "mimeio.h"

#include <kimap/rfccodecs.h>
using namespace KIMAP;

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


  QByteArray outputParameter (Q3Dict < QString > *);

//  int parsePart (mimeIO &, const QString&);
//  int parseBody (mimeIO &, QByteArray &, const QString&, bool mbox = false);

  // parse a header. returns true if it had a leading 'From ' line
  bool parseHeader (mimeIO &);

  QString getDispositionParm (const QByteArray&);
  void setDispositionParm (const QByteArray&, const QString&);
  Q3DictIterator < QString > getDispositionIterator ();

  QString getTypeParm (const QByteArray&);
  void setTypeParm (const QByteArray&, const QString&);
  Q3DictIterator < QString > getTypeIterator ();

  // recursively serialize all important contents to the QDataStream
  void serialize(QDataStream& stream);

  const QByteArray& getType ()
  {
    return _contentType;
  }
  void setType (const QByteArray& _str)
  {
    _contentType = _str;
  }

  const QByteArray& getDescription ()
  {
    return _contentDescription;
  }
  void setDescription( const QByteArray& _str )
  {
    _contentDescription = _str;
  }

  const QByteArray& getDisposition ()
  {
    return _contentDisposition;
  }
  void setDisposition( const QByteArray& _str )
  {
    _contentDisposition = _str;
  }

  const QByteArray& getEncoding ()
  {
    return _contentEncoding;
  }
  void setEncoding (const QByteArray &_str )
  {
    _contentEncoding = _str;
  }

  const QByteArray& getMD5 ()
  {
    return _contentMD5;
  }
  void setMD5 (const QByteArray & _str)
  {
    _contentMD5 = _str;
  }

  const QByteArray& getID ()
  {
    return _contentID;
  }
  void setID (const QByteArray & _str)
  {
    _contentID = _str;
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
  void setContent (const QByteArray &aContent)
  {
    mimeContent = aContent;
  }
  QByteArray getContent ()
  {
    return mimeContent;
  }

  QByteArray getBody ()
  {
    return preMultipartBody + postMultipartBody;
  }
  QByteArray getPreBody ()
  {
    return preMultipartBody;
  }
  void setPreBody (QByteArray & inBody)
  {
    preMultipartBody = inBody;
  }

  QByteArray getPostBody ()
  {
    return postMultipartBody;
  }
  void setPostBody (QByteArray & inBody)
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
    contentType = QByteArray (_str.toLatin1 ()) + "/" + subtypeStr ().toLatin1 ();
  }
  QString subtypeStr ()
  {
    return QString (contentType.
                    right (contentType.length () - contentType.find ('/') -
                           1));
  }
  void setSubtypeStr (const QString & _str)
  {
    contentType = QByteArray (typeStr ().toLatin1 ()) + "/" + _str.toLatin1 ();
  }
  QString cteStr ()
  {
    return QString (getEncoding ());
  }
  void setCteStr (const QString & _str)
  {
    setEncoding (_str.toLatin1 ());
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
    return QString (RfcCodecs::decodeRFC2047String (_contentDescription));
  }
  void setContentDescription (const QString & _str)
  {
    _contentDescription = RfcCodecs::encodeRFC2047String (_str).toLatin1 ();
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
    setDisposition (_str.toLatin1 ());
  }
#endif

protected:
  static void addParameter (const QByteArray&, Q3Dict < QString > *);
  static QString getParameter (const QByteArray&, Q3Dict < QString > *);
  static void setParameter (const QByteArray&, const QString&, Q3Dict < QString > *);

  Q3PtrList < mimeHdrLine > originalHdrLines;

private:
  Q3PtrList < mimeHdrLine > additionalHdrLines;
  Q3Dict < QString > typeList;
  Q3Dict < QString > dispositionList;
  QByteArray _contentType;
  QByteArray _contentDisposition;
  QByteArray _contentEncoding;
  QByteArray _contentDescription;
  QByteArray _contentID;
  QByteArray _contentMD5;
  unsigned int contentLength;
  QByteArray mimeContent;
  QByteArray preMultipartBody;
  QByteArray postMultipartBody;
  mimeHeader *nestedMessage;
  Q3PtrList < mimeHeader > nestedParts;
  QString partSpecifier;

};

#endif
