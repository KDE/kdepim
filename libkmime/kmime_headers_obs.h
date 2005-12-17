/*
    kmime_headers.h

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
#ifndef __KMIME_HEADERS_OBS_H__
#define __KMIME_HEADERS_OBS_H__

#if defined(KMIME_NEW_STYLE_CLASSTREE)
#error You cannot use this file with the new header classes!
#endif

#include <kdepimmacros.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3PtrList>

/** Represents a "Message-Id" header */
class KDE_EXPORT MessageID : public Base {

  public:
    MessageID() : Base()  {}
    MessageID(Content *p) : Base(p) {}
    MessageID(Content *p, const Q3CString &s) : Base(p) { from7BitString(s); }
    MessageID(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~MessageID()  {}

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString&);
    virtual QString asUnicodeString();
    virtual void clear()            { m_id.resize(0); }
    virtual bool isEmpty()          { return (m_id.isEmpty()); }
    virtual const char* type()      { return "Message-Id"; }

    void generate(const Q3CString &fqdn);

  protected:
    Q3CString m_id;

};

/** Represents a "Supersedes" header */
class KDE_EXPORT Supersedes : public MessageID {

  public:
    Supersedes() : MessageID()  {}
    Supersedes(Content *p) : MessageID(p)  {}
    Supersedes(Content *p, const Q3CString &s) : MessageID(p,s)  {}
    Supersedes(Content *p, const QString &s)  : MessageID(p,s)  {}
    ~Supersedes()                   {}

    virtual const char* type()      { return "Supersedes"; }

};

/** This class encapsulates an address-field, containing
    an email-address and a real name */
class KDE_EXPORT AddressField : public Base {

  public:
    AddressField() : Base()  {}
    AddressField(Content *p) : Base(p)  {}
    AddressField(Content *p, const Q3CString &s) : Base(p)  { from7BitString(s); }
    AddressField(Content *p, const QString &s, const Q3CString &cs) : Base(p)  { fromUnicodeString(s, cs); }
    AddressField(const AddressField &a):  Base(a.p_arent)  { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCS=a.e_ncCS; }
    ~AddressField()  {}

    AddressField& operator=(const AddressField &a)  { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCS=a.e_ncCS; return (*this); }

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString &cs);
    virtual QString asUnicodeString();
    virtual void clear()              { n_ame.truncate(0); e_mail.resize(0); }
    virtual bool isEmpty()            { return (e_mail.isEmpty() && n_ame.isEmpty()); }

    bool hasName()                    { return ( !n_ame.isEmpty() ); }
    bool hasEmail()                   { return ( !e_mail.isEmpty() ); }
    QString name()                    { return n_ame; }
    Q3CString nameAs7Bit();
    Q3CString email()                  { return e_mail; }
    void setName(const QString &s)    { n_ame=s; }
    void setNameFrom7Bit(const Q3CString &s);
    void setEmail(const Q3CString &s)  { e_mail=s; }

  protected:
    QString n_ame;
    Q3CString e_mail;
};
typedef QList<AddressField*> ObsAddressList;

/** Represent a "From" header */
class KDE_EXPORT From : public AddressField {

  public:
    From() : AddressField()  {}
    From(Content *p) : AddressField(p)  {}
    From(Content *p, const Q3CString &s) : AddressField(p,s)  {}
    From(Content *p, const QString &s, const Q3CString &cs) : AddressField(p,s,cs)  {}
    ~From()  {}

    virtual const char* type()      { return "From"; }
};


/** Represents a "Reply-To" header */
class KDE_EXPORT ReplyTo : public AddressField {

  public:
    ReplyTo() : AddressField()  {}
    ReplyTo(Content *p) : AddressField(p)  {}
    ReplyTo(Content *p, const Q3CString &s) : AddressField(p,s)  {}
    ReplyTo(Content *p, const QString &s, const Q3CString &cs) : AddressField(p,s,cs)  {}
    ~ReplyTo()  {}

    virtual const char* type()      { return "Reply-To"; }

};


/** Represents a "Mail-Copies-To" header
    http://www.newsreaders.com/misc/mail-copies-to.html */
class KDE_EXPORT MailCopiesTo : public AddressField {

  public:
    MailCopiesTo() : AddressField()  {}
    MailCopiesTo(Content *p) : AddressField(p)  {}
    MailCopiesTo(Content *p, const Q3CString &s) : AddressField(p,s)  {}
    MailCopiesTo(Content *p, const QString &s, const Q3CString &cs) : AddressField(p,s,cs)  {}
    ~MailCopiesTo()  {}

    bool isValid();
    bool alwaysCopy();
    bool neverCopy();

    virtual const char* type()      { return "Mail-Copies-To"; }

};

/** Represents a "To" header */
class KDE_EXPORT To : public Base {

  public:
    To() : Base()  {}
    To(Content *p) : Base(p)  {}
    To(Content *p, const Q3CString &s) : Base(p) { from7BitString(s); }
    To(Content *p, const QString &s, const Q3CString &cs) : Base(p)  { fromUnicodeString(s,cs); }
    ~To()  { qDeleteAll( a_ddrList ); a_ddrList.clear(); }

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString &cs);
    virtual QString asUnicodeString();
    virtual void clear()            { qDeleteAll( a_ddrList ); a_ddrList.clear(); }
    virtual bool isEmpty()          { return a_ddrList.isEmpty() || a_ddrList.first()->isEmpty(); }
    virtual const char* type()      { return "To"; }

    void addAddress(const AddressField &a);
    QList<QByteArray> emails() const;

  protected:
    ObsAddressList a_ddrList;

};


/** Represents a "CC" header */
class KDE_EXPORT CC : public To {

  public:
    CC() : To()  {}
    CC(Content *p) : To(p)  {}
    CC(Content *p, const Q3CString &s) : To(p,s)  {}
    CC(Content *p, const QString &s, const Q3CString &cs) : To(p,s,cs)  {}
    ~CC()  {}

    virtual const char* type()      { return "CC"; }

};


/** Represents a "BCC" header */
class KDE_EXPORT BCC : public To {

  public:
    BCC() : To()  {}
    BCC(Content *p) : To(p)  {}
    BCC(Content *p, const Q3CString &s) : To(p,s)  {}
    BCC(Content *p, const QString &s, const Q3CString &cs) : To(p,s,cs)  {}
    ~BCC()  {}

    virtual const char* type()      { return "BCC"; }

};

/** Represents a "References" header */
class KDE_EXPORT References : public Base {

  public:
    References() : Base(),p_os(-1)  {}
    References(Content *p) : Base(p),p_os(-1)  {}
    References(Content *p, const Q3CString &s) : Base(p),p_os(-1)  { from7BitString(s); }
    References(Content *p, const QString &s) : Base(p),p_os(-1)  { fromUnicodeString(s, Latin1); }
    ~References()                 {}

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString&);
    virtual QString asUnicodeString();
    virtual void clear()            { r_ef.resize(0); p_os=0; }
    virtual bool isEmpty()          { return (r_ef.isEmpty()); }
    virtual const char* type()      { return "References"; }

    int count();
    Q3CString first();
    Q3CString next();
    Q3CString at(unsigned int i);
    void append(const Q3CString &s);

  protected:
    Q3CString r_ef;
    int p_os;

};

/** Represents a "Content-Type" header */
class KDE_EXPORT ContentType : public Base {

  public:
    ContentType() : Base(),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p) : Base(p),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p, const Q3CString &s) : Base(p)  { from7BitString(s); }
    ContentType(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~ContentType()  {}

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString&);
    virtual QString asUnicodeString();
    virtual void clear()            { m_imeType.resize(0); p_arams.resize(0); }
    virtual bool isEmpty()          { return (m_imeType.isEmpty()); }
    virtual const char* type()      { return "Content-Type"; }


    //mime-type handling
    Q3CString mimeType()                     { return m_imeType; }
    Q3CString mediaType();
    Q3CString subType();
    void setMimeType(const Q3CString &s);
    bool isMediatype(const char *s);
    bool isSubtype(const char *s);
    bool isText();
    bool isPlainText();
    bool isHTMLText();
    bool isImage();
    bool isMultipart();
    bool isPartial();

    //parameter handling
    Q3CString charset();
    void setCharset(const Q3CString &s);
    Q3CString boundary();
    void setBoundary(const Q3CString &s);
    QString name();
    void setName(const QString &s, const Q3CString &cs);
    Q3CString id();
    void setId(const Q3CString &s);
    int partialNumber();
    int partialCount();
    void setPartialParams(int total, int number);

    //category
    contentCategory category()            { return c_ategory; }
    void setCategory(contentCategory c)   { c_ategory=c; }

  protected:
    Q3CString getParameter(const char *name);
    void setParameter(const Q3CString &name, const Q3CString &value, bool doubleQuotes=false);
    QByteArray m_imeType, p_arams;
    contentCategory c_ategory;

};


/** Represents a "Content-Transfer-Encoding" header */
class KDE_EXPORT CTEncoding : public Base {

  public:
    CTEncoding() : Base(),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p) : Base(p),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p, const Q3CString &s) : Base(p)  { from7BitString(s); }
    CTEncoding(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~CTEncoding()  {}

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString&);
    virtual QString asUnicodeString();
    virtual void clear()            { d_ecoded=true; c_te=CE7Bit; }
    virtual const char* type()      { return "Content-Transfer-Encoding"; }

    contentEncoding cte()                   { return c_te; }
    void setCte(contentEncoding e)          { c_te=e; }
    bool decoded()                          { return d_ecoded; }
    void setDecoded(bool d=true)            { d_ecoded=d; }
    bool needToEncode()                     { return (d_ecoded && (c_te==CEquPr || c_te==CEbase64)); }

  protected:
    contentEncoding c_te;
    bool d_ecoded;

};


/** Represents a "Content-Disposition" header */
class KDE_EXPORT CDisposition : public Base {

  public:
    CDisposition() : Base(),d_isp(CDinline)  {}
    CDisposition(Content *p) : Base(p),d_isp(CDinline)  {}
    CDisposition(Content *p, const Q3CString &s) : Base(p)  { from7BitString(s); }
    CDisposition(Content *p, const QString &s, const Q3CString &cs) : Base(p)  { fromUnicodeString(s, cs); }
    ~CDisposition()  {}

    virtual void from7BitString(const Q3CString &s);
    virtual Q3CString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const Q3CString &cs);
    virtual QString asUnicodeString();
    virtual void clear()            { f_ilename.truncate(0); d_isp=CDinline; }
    virtual const char* type()      { return "Content-Disposition"; }

    contentDisposition disposition()          { return d_isp; }
    void setDisposition(contentDisposition d) { d_isp=d; }
    bool isAttachment()                       { return (d_isp==CDattachment); }

    QString filename()                        { return f_ilename; }
    void setFilename(const QString &s)        { f_ilename=s; }

  protected:
    contentDisposition d_isp;
    QString f_ilename;

};


/** Represents a "Content-Description" header */
class KDE_EXPORT CDescription : public Generics::GUnstructured {

  public:
    CDescription() : Generics::GUnstructured()  {}
    CDescription( Content * p ) : Generics::GUnstructured( p )  {}
    CDescription( Content * p, const Q3CString & s )
      : Generics::GUnstructured( p, s ) {};
    CDescription( Content * p, const QString & s, const Q3CString & cs )
      : Generics::GUnstructured( p, s, cs ) {}
    ~CDescription()  {}

    virtual const char* type()      { return "Content-Description"; }
};

#endif  // __KMIME_HEADERS_OBS_H__
