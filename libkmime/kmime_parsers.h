/*
    kmime_parsers.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#ifndef __KMIME_PARSERS__
#define __KMIME_PARSERS__

#include <qvaluelist.h>
#include <qcstring.h>
#include <qstrlist.h>

namespace KMime {

namespace Parser {

/** Helper-class: splits a multipart-message into single
    parts as described in RFC 2046
    @internal
*/
class MultiPart {
  
public:
  MultiPart(const QCString &src, const QCString &boundary);
  ~MultiPart() {};
  
  bool parse();
  QValueList<QCString> parts()    { return p_arts; }
  QCString preamble()     { return p_reamble; }
  QCString epilouge()     { return e_pilouge; }
  
protected:
  QCString s_rc, b_oundary, p_reamble, e_pilouge;
  QValueList<QCString> p_arts;
};


/** Helper-class: abstract base class of all parsers for
    non-mime binary data (uuencoded, yenc)
    @internal
*/
class NonMimeParser {

public:
  NonMimeParser(const QCString &src);
  virtual ~NonMimeParser() {};
  virtual bool parse() = 0;
  bool isPartial()            { return (p_artNr>-1 && t_otalNr>-1 && t_otalNr!=1); }
  int partialNumber()         { return p_artNr; }
  int partialCount()          { return t_otalNr; }
  bool hasTextPart()          { return (t_ext.length()>1); }
  QCString textPart()         { return t_ext; }
  QStrList binaryParts()       { return b_ins; }
  QStrList filenames()         { return f_ilenames; }
  QStrList mimeTypes()         { return m_imeTypes; }

protected:
  static QCString guessMimeType(const QCString& fileName);

  QCString s_rc, t_ext;
  QStrList b_ins, f_ilenames, m_imeTypes;
  int p_artNr, t_otalNr;
};


/** Helper-class: tries to extract the data from a possibly
    uuencoded message
    @internal
*/
class UUEncoded : public NonMimeParser {

public:
  UUEncoded(const QCString &src, const QCString &subject);  

  virtual bool parse();

protected:
  QCString s_ubject;  
};



/** Helper-class: tries to extract the data from a possibly
    yenc encoded message
    @internal
*/
class YENCEncoded : public NonMimeParser {

public:
  YENCEncoded(const QCString &src);  

  virtual bool parse();      
  QValueList<QByteArray> binaryParts()       { return b_ins; }
    
protected:
  QValueList<QByteArray> b_ins;
  static bool yencMeta( QCString& src, const QCString& name, int* value);
};


} // namespace Parser

} // namespace KMime

#endif // __KMIME_PARSERS__
