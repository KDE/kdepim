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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#ifndef __KMIME_PARSERS__
#define __KMIME_PARSERS__

#include <tqvaluelist.h>
#include <tqcstring.h>
#include <tqstrlist.h>

namespace KMime {

namespace Parser {

/** Helper-class: splits a multipart-message into single
    parts as described in RFC 2046
    @internal
*/
class MultiPart {
  
public:
  MultiPart(const TQCString &src, const TQCString &boundary);
  ~MultiPart() {};
  
  bool parse();
  TQValueList<TQCString> parts()    { return p_arts; }
  TQCString preamble()     { return p_reamble; }
  TQCString epilouge()     { return e_pilouge; }
  
protected:
  TQCString s_rc, b_oundary, p_reamble, e_pilouge;
  TQValueList<TQCString> p_arts;
};


/** Helper-class: abstract base class of all parsers for
    non-mime binary data (uuencoded, yenc)
    @internal
*/
class NonMimeParser {

public:
  NonMimeParser(const TQCString &src);
  virtual ~NonMimeParser() {};
  virtual bool parse() = 0;
  bool isPartial()            { return (p_artNr>-1 && t_otalNr>-1 && t_otalNr!=1); }
  int partialNumber()         { return p_artNr; }
  int partialCount()          { return t_otalNr; }
  bool hasTextPart()          { return (t_ext.length()>1); }
  TQCString textPart()         { return t_ext; }
  TQStrList binaryParts()       { return b_ins; }
  TQStrList filenames()         { return f_ilenames; }
  TQStrList mimeTypes()         { return m_imeTypes; }

protected:
  static TQCString guessMimeType(const TQCString& fileName);

  TQCString s_rc, t_ext;
  TQStrList b_ins, f_ilenames, m_imeTypes;
  int p_artNr, t_otalNr;
};


/** Helper-class: tries to extract the data from a possibly
    uuencoded message
    @internal
*/
class UUEncoded : public NonMimeParser {

public:
  UUEncoded(const TQCString &src, const TQCString &subject);  

  virtual bool parse();

protected:
  TQCString s_ubject;  
};



/** Helper-class: tries to extract the data from a possibly
    yenc encoded message
    @internal
*/
class YENCEncoded : public NonMimeParser {

public:
  YENCEncoded(const TQCString &src);  

  virtual bool parse();      
  TQValueList<TQByteArray> binaryParts()       { return b_ins; }
    
protected:
  TQValueList<TQByteArray> b_ins;
  static bool yencMeta( TQCString& src, const TQCString& name, int* value);
};


} // namespace Parser

} // namespace KMime

#endif // __KMIME_PARSERS__
