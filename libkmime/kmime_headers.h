/*  -*- c++ -*
    kmime_headers.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0 as
    published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#ifndef __KMIME_HEADERS_H__
#define __KMIME_HEADERS_H__

// Content:
//
// - header's base class defining the common interface
// - generic base classes for different types of fields
// - incompatible, GStructured-based field classes
// - compatible, GUnstructured-based field classes

#include "kmime_header_parsing.h"

#include <qstring.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qasciidict.h>
#include <qmap.h>
#include <qptrlist.h>

#include <time.h>

#include <kdemacros.h>

namespace KMime {

//forward declaration
class Content;

namespace Headers {


enum contentCategory    { CCsingle,
                          CCcontainer,
                          CCmixedPart,
                          CCalternativePart };

enum contentEncoding    { CE7Bit,
                          CE8Bit,
                          CEquPr,
                          CEbase64,
                          CEuuenc,
                          CEbinary };

enum contentDisposition { CDinline,
                          CDattachment,
                          CDparallel };

//often used charset
static const QCString Latin1("ISO-8859-1");

#define mk_trivial_subclass_with_name( subclass, subclassName, baseclass ) \
class subclass : public Generics::baseclass { \
public: \
  subclass() : Generics::baseclass() {} \
  subclass( Content * p ) : Generics::baseclass( p ) {} \
  subclass( Content * p, const QCString & s ) \
    : Generics::baseclass( p ) { from7BitString( s ); } \
  subclass( Content * p, const QString & s, const QCString & cs ) \
    : Generics::baseclass( p ) { fromUnicodeString( s, cs ); } \
  ~subclass() {} \
  \
  const char * type() const { return #subclassName; } \
}
 
#define mk_trivial_subclass( subclass, baseclass ) \
mk_trivial_subclass_with_name( subclass, subclass, baseclass )

#define mk_parsing_subclass_with_name( subclass, subclassName, baseclass ) \
class subclass : public Generics::baseclass { \
public: \
  subclass() : Generics::baseclass() {} \
  subclass( Content * p ) : Generics::baseclass( p ) {} \
  subclass( Content * p, const QCString & s ) \
    : Generics::baseclass( p ) { from7BitString( s ); } \
  subclass( Content * p, const QString & s, const QCString & cs ) \
    : Generics::baseclass( p ) { fromUnicodeString( s, cs ); } \
  ~subclass() {} \
  \
  const char * type() const { return #subclassName; } \
protected: \
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false ); \
}

#define mk_parsing_subclass( subclass, baseclass ) \
mk_parsing_subclass_with_name( subclass, subclass, baseclass )

//
//
// HEADER'S BASE CLASS. DEFINES THE COMMON INTERFACE
//
//

/** Baseclass of all header-classes. It represents a
    header-field as described in RFC-822.  */
class KDE_EXPORT Base {

  public:
    typedef QPtrList<Base> List;

    /** Create an empty header. */
    Base() : e_ncCS(0), p_arent(0) {}

    /** Create an empty header with a parent-content. */
    Base(KMime::Content *parent) : e_ncCS(0), p_arent(parent) {}

    /** Destructor */
    virtual ~Base()  {}

    /** Return the parent of this header. */
    KMime::Content* parent()  { return p_arent; }

    /** Set the parent for this header. */
    void setParent(KMime::Content *p)  { p_arent=p; }

    /** Parse the given string. Take care of RFC2047-encoded
	strings. A default charset is given. If the last parameter
	is true the default charset is used in any case */
    virtual void from7BitString(const QCString&)  {}

    /** Return the encoded header. The parameter specifies
	whether the header-type should be included. */
    virtual QCString as7BitString(bool=true)  { return QCString(); }

    /** Return the charset that is used for RFC2047-encoding */
    QCString rfc2047Charset();

    /** Set the charset for RFC2047-encoding */
    void setRFC2047Charset(const QCString &cs);

    /** Return the default charset */
    QCString defaultCS();

    /** Return if the default charset is mandatory */
    bool forceCS();

    /** Parse the given string and set the charset. */
    virtual void fromUnicodeString(const QString&, const QCString&)  {}

    /** Return the decoded content of the header without
       the header-type. */
    virtual QString asUnicodeString()  { return QString(); }

    /** Delete */
    virtual void clear()  {}

    /** Do we have data? */
    virtual bool isEmpty()  { return false; }

    /** Return the type of this header (e.g. "From") */
    virtual const char* type()  { return ""; }

    /** Check if this header is of type t. */
    bool is(const char* t)  { return (strcasecmp(t, type())==0); }

    /** Check if this header is a MIME header */
    bool isMimeHeader()  { return (strncasecmp(type(), "Content-", 8)==0); }

    /** Check if this header is a X-Header */
    bool isXHeader()  { return (strncmp(type(), "X-", 2)==0); }

  protected:
    QCString typeIntro()  { return (QCString(type())+": "); }

    const char *e_ncCS;
    Content *p_arent;

};


//
//
// GENERIC BASE CLASSES FOR DIFFERENT TYPES OF FIELDS
//
//

namespace Generics {

/** Abstract base class for unstructured header fields
    (e.g. "Subject", "Comment", "Content-description").
    
    Features: Decodes the header according to RFC2047, incl. RFC2231
    extensions to encoded-words.

    Subclasses need only re-implement @p const @p char* @p type().

    A macro to automate this is named
\code
    MK_TRIVIAL_GUnstructured_SUBCLASS(classname,headername);
\endcode

    The ContentDescription class then reads:
\code
    MK_TRIVIAL_GUnstructured_SUBCLASS(ContentDescription,Content-Description);
\endcode
*/

  // known issues:
  // - uses old decodeRFC2047String function, instead of our own...

class KDE_EXPORT GUnstructured : public Base {

public:
  GUnstructured() : Base()  {}
  GUnstructured( Content * p ) : Base( p ) {}
  GUnstructured( Content * p, const QCString & s )
    : Base( p ) { from7BitString(s); }
  GUnstructured( Content * p, const QString & s, const QCString & cs )
    : Base( p )  { fromUnicodeString( s, cs ); }
  ~GUnstructured()  {}

  virtual void from7BitString( const QCString& str );
  virtual QCString as7BitString( bool withHeaderType=true );

  virtual void fromUnicodeString( const QString & str,
				  const QCString & suggestedCharset);
  virtual QString asUnicodeString();

  virtual void clear()            { d_ecoded.truncate(0); }
  virtual bool isEmpty()          { return (d_ecoded.isEmpty()); }

private:
  QString d_ecoded;
};

/** This is the base class for all structured header fields. It
    contains parsing methods for all basic token types found in
    rfc2822.

    @section Parsing

    At the basic level, there are tokens & tspecials (rfc2045),
    atoms & specials, quoted-strings, domain-literals (all rfc822) and
    encoded-words (rfc2047).

    As a special token, we have the comment. It is one of the basic
    tokens defined in rfc822, but it's parsing relies in part on the
    basic token parsers (e.g. comments may contain encoded-words).
    Also, most upper-level parsers (notably those for phrase and
    dot-atom) choose to ignore any comment when parsing.

    Then there are the real composite tokens, which are made up of one
    or more of the basic tokens (and semantically invisible comments):
    phrases (rfc822 with rfc2047) and dot-atoms (rfc2822).
    
    This finishes the list of supported token types. Subclasses will
    provide support for more higher-level tokens, where necessary,
    using these parsers.

    @short Base class for structured header fields.
    @author Marc Mutz <mutz@kde.org>
*/

class KDE_EXPORT GStructured : public Base {
public:
  GStructured() : Base()  {}
  GStructured( Content * p ) : Base( p ) {}
  GStructured( Content * p, const QCString & s )
    : Base( p ) { from7BitString(s); }
  GStructured( Content * p, const QString & s, const QCString & cs )
    : Base( p )  { fromUnicodeString( s, cs ); }
  ~GStructured()  {}

  
protected:
#if 0
  // the assembly squad:

  bool writeAtom( char* & dcursor, const char * const dend, const QString & input );
  bool writeAtom( char* & dcursor, const char * const dend,
		  const QPair<const char*,int> & input );
  bool writeToken( char* & dcursor, const char * const dend, const QString & input );
  bool writeToken( char* & dcursor, const char * const dend,
		   const QPair<const char*int> & input );

  bool writeGenericQuotedString( char* & dcursor, const char * const dend,
				 const QString & input, bool withCRLF=false );
  bool writeComment( char* & dcursor, const char * const dend,
		     const QString & input, bool withCRLF=false );
  bool writePhrase( char* & dcursor, const char * const dend,
		    const QString & input, bool withCRLF=false );
  bool writeDotAtom( char* & dcursor, const char * const dend,
		     const QString & input, bool withCRLF=false );
#endif
};


class KDE_EXPORT GAddress : public GStructured {
public:
  GAddress() : GStructured()  {}
  GAddress( Content * p ) : GStructured( p ) {}
  GAddress( Content * p, const QCString & s )
    : GStructured( p ) { from7BitString(s); }
  GAddress( Content * p, const QString & s, const QCString & cs )
    : GStructured( p )  { fromUnicodeString( s, cs ); }
  ~GAddress()  {}

protected:
};


/** Base class for headers that deal with (possibly multiple)
    addresses, but don't allow groups: */
class KDE_EXPORT MailboxList : public GAddress {
public:
  MailboxList() : GAddress()  {}
  MailboxList( Content * p ) : GAddress( p ) {}
  MailboxList( Content * p, const QCString & s )
    : GAddress( p ) { from7BitString(s); }
  MailboxList( Content * p, const QString & s, const QCString & cs )
    : GAddress( p )  { fromUnicodeString( s, cs ); }
  ~MailboxList()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  /** The list of mailboxes */
  QValueList<Types::Mailbox> mMailboxList;
};


/** Base class for headers that deal with exactly one mailbox
    (e.g. Sender) */
mk_parsing_subclass(SingleMailbox,MailboxList);

/** Base class for headers that deal with (possibly multiple)
    addresses, allowing groups. */
class KDE_EXPORT AddressList : public GAddress {
public:
  AddressList() : GAddress()  {}
  AddressList( Content * p ) : GAddress( p ) {}
  AddressList( Content * p, const QCString & s )
    : GAddress( p ) { from7BitString(s); }
  AddressList( Content * p, const QString & s, const QCString & cs )
    : GAddress( p )  { fromUnicodeString( s, cs ); }
  ~AddressList()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  /** The list of addresses */
  QValueList<Types::Address> mAddressList;
};

/** Base class for headers which deal with a list of msg-id's */
class KDE_EXPORT GIdent : public GAddress {
public:
  GIdent() : GAddress()  {}
  GIdent( Content * p ) : GAddress( p ) {}
  GIdent( Content * p, const QCString & s )
    : GAddress( p ) { from7BitString(s); }
  GIdent( Content * p, const QString & s, const QCString & cs )
    : GAddress( p )  { fromUnicodeString( s, cs ); }
  ~GIdent()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  /** The list of msg-id's */
  QValueList<Types::AddrSpec> mMsgIdList;
};

/** Base class for headers which deal with a list of msg-id's */
mk_parsing_subclass(GSingleIdent,GIdent);

/** Base class for headers which deal with a single atom. */
class KDE_EXPORT GToken : public GStructured {
public:
  GToken() : GStructured()  {}
  GToken( Content * p ) : GStructured( p ) {}
  GToken( Content * p, const QCString & s )
    : GStructured( p ) { from7BitString(s); }
  GToken( Content * p, const QString & s, const QCString & cs )
    : GStructured( p )  { fromUnicodeString( s, cs ); }
  ~GToken()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  QCString mToken;
};


class KDE_EXPORT GPhraseList : public GStructured {
public:
  GPhraseList() : GStructured()  {}
  GPhraseList( Content * p ) : GStructured( p ) {}
  GPhraseList( Content * p, const QCString & s )
    : GStructured( p ) { from7BitString(s); }
  GPhraseList( Content * p, const QString & s, const QCString & cs )
    : GStructured( p )  { fromUnicodeString( s, cs ); }
  ~GPhraseList()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  QStringList mPhraseList;
};

class KDE_EXPORT GDotAtom : public GStructured {
public:
  GDotAtom() : GStructured()  {}
  GDotAtom( Content * p ) : GStructured( p ) {}
  GDotAtom( Content * p, const QCString & s )
    : GStructured( p ) { from7BitString(s); }
  GDotAtom( Content * p, const QString & s, const QCString & cs )
    : GStructured( p )  { fromUnicodeString( s, cs ); }
  ~GDotAtom()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  QString mDotAtom;
};

class KDE_EXPORT GParametrized : public GStructured {
public:
  GParametrized() : GStructured()  {}
  GParametrized( Content * p ) : GStructured( p ) {}
  GParametrized( Content * p, const QCString & s )
    : GStructured( p ) { from7BitString(s); }
  GParametrized( Content * p, const QString & s, const QCString & cs )
    : GStructured( p )  { fromUnicodeString( s, cs ); }
  ~GParametrized()  {}

protected:
  QMap<QString,QString> mParameterHash;

private:
};

class KDE_EXPORT GContentType : public GParametrized {
public:
  GContentType() : GParametrized()  {}
  GContentType( Content * p ) : GParametrized( p ) {}
  GContentType( Content * p, const QCString & s )
    : GParametrized( p ) { from7BitString(s); }
  GContentType( Content * p, const QString & s, const QCString & cs )
    : GParametrized( p )  { fromUnicodeString( s, cs ); }
  ~GContentType()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  QCString mMimeType;
  QCString mMimeSubType;
};


class KDE_EXPORT GCISTokenWithParameterList : public GParametrized {
public:
  GCISTokenWithParameterList() : GParametrized()  {}
  GCISTokenWithParameterList( Content * p ) : GParametrized( p ) {}
  GCISTokenWithParameterList( Content * p, const QCString & s )
    : GParametrized( p ) { from7BitString(s); }
  GCISTokenWithParameterList( Content * p, const QString & s, const QCString & cs )
    : GParametrized( p )  { fromUnicodeString( s, cs ); }
  ~GCISTokenWithParameterList()  {}

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

  QCString mToken;
};


} // namespace Generics

//
//
// INCOMPATIBLE, GSTRUCTURED-BASED FIELDS:
//
//


/** Represents the Return-Path header field. */
class KDE_EXPORT ReturnPath : public Generics::GAddress {
public:
  ReturnPath() : Generics::GAddress()  {}
  ReturnPath( Content * p ) : Generics::GAddress( p ) {}
  ReturnPath( Content * p, const QCString & s )
    : Generics::GAddress( p ) { from7BitString(s); }
  ReturnPath( Content * p, const QString & s, const QCString & cs )
    : Generics::GAddress( p )  { fromUnicodeString( s, cs ); }
  ~ReturnPath()  {}

  const char * type() const { return "Return-Path"; }

protected:
  bool parse( const char* & scursor, const char * const send, bool isCRLF=false );
};

#if defined(KMIME_NEW_STYLE_CLASSTREE)
// classes whose names collide with earlier ones:

// GAddress et al.:

// rfc(2)822 headers:
mk_trivial_subclass(From,MailboxList);
mk_trivial_subclass(Sender,SingleMailbox);
mk_trivial_subclass_with_name(ReplyTo,Reply-To,AddressList);
mk_trivial_subclass(Cc,AddressList);
mk_trivial_subclass(Bcc,AddressList);
// usefor headers:
mk_trivial_subclass_with_name(MailCopiesTo,Mail-Copies-To,AddressList);

// GToken:

mk_trivial_subclass_with_name(ContentTransferEncoding,
			      Content-Transfer-Encoding,GToken);

// GPhraseList:

mk_trivial_subclass(Keywords,GPhraseList);

// GDotAtom:

mk_trivial_subclass_with_name(MIMEVersion,MIME-Version,GDotAtom);

// GIdent:

mk_trivial_subclass_with_name(MessageID,Message-ID,GSingleIdent);
mk_trivial_subclass_with_name(ContentID,Content-ID,GSingleIdent);
mk_trivial_subclass(Supersedes,GSingleIdent);
mk_trivial_subclass_with_name(InReplyTo,In-Reply-To,GIdent);
mk_trivial_subclass(References,GIdent);

// GContentType:

mk_trivial_subclass_with_name(ContentType,ContentType,GContentType);

// GCISTokenWithParameterList:

mk_trivial_subclass_with_name(ContentDisposition,Content-Disposition,
			      GCISTokenWithParameterList);


#endif


//
//
// COMPATIBLE GUNSTRUCTURED-BASED FIELDS:
//
//


/** Represents an arbitrary header, that can contain
    any header-field.
    Adds a type over GUnstructured.
    @see GUnstructured
*/
class KDE_EXPORT Generic : public Generics::GUnstructured {

  public:
    Generic() : Generics::GUnstructured(), t_ype(0) {}
    Generic(const char *t)
      : Generics::GUnstructured(), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p)
      : Generics::GUnstructured( p ), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p, const QCString &s)
      : Generics::GUnstructured( p, s ), t_ype(0) { setType(t); }
    Generic(const char *t, Content *p, const QString &s, const QCString &cs)
      : Generics::GUnstructured( p, s, cs ), t_ype(0) { setType(t); }
    ~Generic() { delete[] t_ype; }

    virtual void clear()            { delete[] t_ype; GUnstructured::clear(); }
    virtual bool isEmpty()          { return (t_ype==0 || GUnstructured::isEmpty()); }
    virtual const char* type()      { return t_ype; }
    void setType(const char *type);

  protected:
    char *t_ype;

};


/** Represents a "Subject" header */
class KDE_EXPORT Subject : public Generics::GUnstructured {

  public:
    Subject() : Generics::GUnstructured()  {}
    Subject( Content * p ) : Generics::GUnstructured( p )  {}
    Subject( Content * p, const QCString & s )
      : Generics::GUnstructured( p, s ) {}
    Subject( Content * p, const QString & s, const QCString & cs )
      : Generics::GUnstructured( p, s, cs ) {}
    ~Subject()  {}

    virtual const char* type() { return "Subject"; }

    bool isReply() {
      return ( asUnicodeString().find( QString("Re:"), 0, false ) == 0 );
    }
};

/** Represents a "Organization" header */
class KDE_EXPORT Organization : public Generics::GUnstructured {

  public:
    Organization() : Generics::GUnstructured() {}
    Organization( Content * p ) : Generics::GUnstructured( p ) {}
    Organization( Content * p, const QCString & s )
      : Generics::GUnstructured( p, s ) {};
    Organization( Content * p, const QString & s, const QCString & cs)
      : Generics::GUnstructured( p, s, cs ) {}
    ~Organization()  {}

    virtual const char* type()      { return "Organization"; }

};

//
//
// NOT YET CONVERTED STUFF BELOW:
//
//



/** Represents a "Control" header */
class KDE_EXPORT Control : public Base {

  public:
    Control() : Base()  {}
    Control(Content *p) : Base(p)  {}
    Control(Content *p, const QCString &s) : Base(p) { from7BitString(s); }
    Control(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Control()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { c_trlMsg.truncate(0); }
    virtual bool isEmpty()          { return (c_trlMsg.isEmpty()); }
    virtual const char* type()      { return "Control"; }

    bool isCancel()                 { return (c_trlMsg.find("cancel", 0, false)!=-1); }

  protected:
    QCString c_trlMsg;

};

/** Represents a "Date" header */
class KDE_EXPORT Date : public Base {

  public:
    Date() : Base(), t_ime(0)  {}
    Date(Content *p) : Base(p), t_ime(0)  {}
    Date(Content *p, time_t t) : Base(p), t_ime(t)  {}
    Date(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Date(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Date()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { t_ime=0; }
    virtual bool isEmpty()          { return (t_ime==0); }
    virtual const char* type()      { return "Date"; }

    time_t unixTime()               { return t_ime; }
    void setUnixTime(time_t t)      { t_ime=t; }
    void setUnixTime()              { t_ime=time(0); }
    QDateTime qdt();
    int ageInDays();
    
  protected:
    time_t t_ime;

};


/** Represents a "Newsgroups" header */
class KDE_EXPORT Newsgroups : public Base {

  public:
    Newsgroups() : Base()  {}
    Newsgroups(Content *p) : Base(p)  {}
    Newsgroups(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Newsgroups(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Newsgroups()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { g_roups.resize(0); }
    virtual bool isEmpty()          { return g_roups.isEmpty(); }
    virtual const char* type()      { return "Newsgroups"; }

    QCString firstGroup();
    bool isCrossposted()            { return ( g_roups.find(',')>-1 ); }
    QStringList getGroups();

  protected:
    QCString g_roups;

};


/** Represents a "Followup-To" header */
class KDE_EXPORT FollowUpTo : public Newsgroups {

  public:
    FollowUpTo() : Newsgroups()  {}
    FollowUpTo(Content *p) : Newsgroups(p)  {}
    FollowUpTo(Content *p, const QCString &s) : Newsgroups(p,s)  {}
    FollowUpTo(Content *p, const QString &s) : Newsgroups(p,s)  {}
    ~FollowUpTo()  {}

    virtual const char* type()        { return "Followup-To"; }

};


/** Represents a "Lines" header */
class KDE_EXPORT Lines : public Base {

  public:
    Lines() : Base(),l_ines(-1)  {}
    Lines(Content *p) : Base(p),l_ines(-1)  {}
    Lines(Content *p, unsigned int i) : Base(p),l_ines(i)  {}
    Lines(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    Lines(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~Lines()                 {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { l_ines=-1; }
    virtual bool isEmpty()          { return (l_ines==-1); }
    virtual const char* type()      { return "Lines"; }

    int numberOfLines()             { return l_ines; }
    void setNumberOfLines(int i)    { l_ines=i; }

  protected:
    int l_ines;

};



/** Represents a "User-Agent" header */
class KDE_EXPORT UserAgent : public Base {

  public:
    UserAgent() : Base()  {}
    UserAgent(Content *p) : Base(p)  {}
    UserAgent(Content *p, const QCString &s) : Base(p)  { from7BitString(s); }
    UserAgent(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~UserAgent()  {}

    virtual void from7BitString(const QCString &s);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QCString&);
    virtual QString asUnicodeString();
    virtual void clear()            { u_agent.resize(0); }
    virtual bool isEmpty()          { return (u_agent.isEmpty()); }
    virtual const char* type()      { return "User-Agent"; }

  protected:
    QCString u_agent;

};


#if !defined(KMIME_NEW_STYLE_CLASSTREE)
#include "kmime_headers_obs.h"
#endif
}  //namespace Headers

#if 0
typedef Headers::Base* (*headerCreator)(void);

/** This is a factory for KMime::Headers. You can create new header
    objects by type with @ref create and @ref upgrade an existing
    @ref Headers::Generic to a specialized header object.

    If you are a header class author, you can register your class
    (let's call it Foo) so:
    <pre>
    
    </pre>    

    @short Factory for KMime::Headers
    @author Marc Mutz <mutz@kde.org>
    @see KMime::Headers::Base KMime::Headers::Generic
*/

class HeaderFactory : public QAsciiDict<headerCreator>
{
private:
  HeaderFactory();
  ~HeaderFactory() {}
  static QAsciiDict

public:
  /** Create a new header object of type @p aType, or a fitting
      generic substitute, if available and known
  */
  static Headers::Base* create( const char* aType )
  {
    if (!s_elf)
      s_elf = new HeaderFactory;
    headerCreator * hc = (*s_elf)[aType];
    if ( !hc )
      return 0;
    else
      return (*hc)();
  }

  /** This is a wrapper around the above function, provided for
      convenience. It differs from the above only in what arguments it
      takes.
  */
  static Headers::Base* create( const QCString& aType )
  {
    return create( aType.data() );
  }

  /** Consume @p aType and build a header object that corresponds to
      the type that @p aType->type() returns.
      @param  aType    generic header to upgrade. This will be deleted
                       if necessary, so don't use references to it after
                       calling this function.
      @return A corresponding header object (if available), or a generic
      object for this kind of header (if known), or @p aType (else).
      @see Headers::Generic create
  */
  static Headers::Base* upgrade( Headers::Generic* aType ) { (void)aType; return new Headers::Base; }

};

#endif

}  //namespace KMime


#endif // __KMIME_HEADERS_H__

