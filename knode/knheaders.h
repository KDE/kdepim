/*
    knheaders.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNHEADERS_H
#define KNHEADERS_H

#include <qstring.h>
#include <time.h>
#include <qstrlist.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qfont.h>


namespace KNHeaders {

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


/* Baseclass of all header-classes. It represents a
   header-field as described in RFC-822.  */
class Base {

  public:
    /* Create an empty header. */
    Base()                                            { e_ncCSet=QFont::ISO_8859_1; }

    virtual ~Base()                                   {}

    /* Parse the given string. Take care of RFC2047-encoded
       strings. A default charset is given. If the last parameter
       is true the default charset is used in any case */
    virtual void from7BitString(const QCString&, QFont::CharSet, bool)  {}

    /* Return the encoded header. The parameter specifies
       wether the header-type should be included. */
    virtual QCString as7BitString(bool incType=true)        { (void)(incType); return QCString(); }

    /* Parse the given string and set the charset. */
    virtual void fromUnicodeString(const QString&, QFont::CharSet)    {}

    /* Returns the charset that is used for RFC2047-encoding */
    QFont::CharSet rfc2047Charset()             { return e_ncCSet; }
    void setRFC2047Charset(QFont::CharSet cs)   { e_ncCSet=cs; }

    /* use the default charset instead of the declared charset */
    void setForceDefaultCS(bool) {}

    /* Return the decoded content of the header without
       the header-type. */
    virtual QString asUnicodeString()                 { return QString(); }

    /* Delete */
    virtual void clear()        {}

    virtual bool isEmpty()      { return false; }

    /* Returns the type of this header (e.g. "From") */
    virtual const char* type()  { return ""; }

    /* Checks if this header is of type t. */
    bool is(const char* t)      { return (strcasecmp(t, type())==0); }

    /* Checks if this header is a MIME header */
    bool isMimeHeader()         { return (strncasecmp(type(), "Content-", 8)==0); }

    /* Checks if this header is a X-Header */
    bool isXHeader()            { return (strncmp(type(), "X-", 2)==0); }

  protected:
    QCString typeIntro()        { return (QCString(type())+": "); }

    QFont::CharSet e_ncCSet;

};
typedef QList<Base> List;


/* Represents an arbitrary header, that can contain
   any header-field */
class Generic : public Base {

  public:
    Generic(const char *t);
    Generic(const char *t, const QCString &s, QFont::CharSet defaultCS, bool force);
    Generic(const char *t, const QString &s, QFont::CharSet cs);
    ~Generic();

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { delete[] t_ype; u_nicode.truncate(0); }
    virtual bool isEmpty()          { return (t_ype==0 || u_nicode.isEmpty()); }
    virtual const char* type()      { return t_ype; }
    void setType(const char *type);

  protected:
    QString u_nicode;
    char *t_ype;

};


/* Represents a "Message-Id" header */
class MessageID : public Base {

  public:
    MessageID()                  {}
    MessageID(const QCString &s) { from7BitString(s,QFont::ISO_8859_1, false); }
    MessageID(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~MessageID()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { m_id.resize(0); }
    virtual bool isEmpty()          { return (m_id.isEmpty()); }
    virtual const char* type()      { return "Message-Id"; }

    void generate(const QCString &fqdn);

  protected:
    QCString m_id;

};


/* Represents a "Control" header */
class Control : public Base {

  public:
    Control()                   {}
    Control(const QCString &s)  { from7BitString(s, QFont::ISO_8859_1, false); }
    Control(const QString &s)   { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~Control()                  {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { c_trlMsg.truncate(0); }
    virtual bool isEmpty()          { return (c_trlMsg.isEmpty()); }
    virtual const char* type()      { return "Control"; }

    bool isCancel()                 { return (c_trlMsg.find("cancel", 0, false)!=-1); }

  protected:
    QCString c_trlMsg;

};


/* Represents a "Supersedes" header */
class Supersedes : public MessageID {

  public:
    Supersedes()                  : MessageID() {}
    Supersedes(const QCString &s) : MessageID(s) {}
    Supersedes(const QString &s)  : MessageID(s) {}
    ~Supersedes()                   {}

    virtual const char* type()      { return "Supersedes"; }

};


/* Represents a "Subject" header */
class Subject : public Base {

  public:
    Subject()                                                        {}
    Subject(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s, defaultCS, force); }
    Subject(const QString &s, QFont::CharSet cs)                     { fromUnicodeString(s, cs); }
    ~Subject()                                                       {}

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { s_ubject.truncate(0); }
    virtual bool isEmpty()          { return (s_ubject.isEmpty()); }
    virtual const char* type()      { return "Subject"; }

    bool isReply()                  { return (s_ubject.find(QString("Re:"), 0, false)==0); }

  protected:
    QString s_ubject;

};


/* This class encapsulates an address-field, containing
   an email-adress and a real name */
class AddressField : public Base {

  public:
    AddressField()                                            {}
    AddressField(const AddressField &a)                       : Base() { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCSet=a.e_ncCSet; }
    AddressField(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s, defaultCS, force); }
    AddressField(const QString &s, QFont::CharSet cs)         { fromUnicodeString(s, cs); }
    ~AddressField()                                           {}

    AddressField& operator=(const AddressField &a)    { n_ame=a.n_ame; e_mail=a.e_mail.copy(); e_ncCSet=a.e_ncCSet; return (*this); }

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { n_ame.truncate(0); e_mail.resize(0); }
    virtual bool isEmpty()          { return (e_mail.isEmpty()); }

    bool hasName()                    { return ( !n_ame.isEmpty() ); }
    bool hasEmail()                   { return ( !e_mail.isEmpty() ); }
    QString name()                    { return n_ame; }
    QCString nameAs7Bit();
    QCString email()                  { return e_mail; }
    void setName(const QString &s)    { n_ame=s; }
    void setNameFrom7Bit(const QCString &s, QFont::CharSet defaultCS, bool force);
    void setEmail(const QCString &s)  { e_mail=s; }

  protected:
    QString n_ame;
    QCString e_mail;
};
typedef QList<AddressField> AddressList;


/* Represent a "From" header */
class From : public AddressField {

  public:
    From()                                            {}
    From(const QCString &s, QFont::CharSet defaultCS, bool force) : AddressField(s,defaultCS,force) {}
    From(const QString &s, QFont::CharSet cs)         : AddressField(s, cs)       {}
    ~From()                                                                       {}

    virtual const char* type()      { return "From"; }
};


/* Represents a "Reply-To" header */
class ReplyTo : public AddressField {

  public:
    ReplyTo()                                            {}
    ReplyTo(const QCString &s, QFont::CharSet defaultCS, bool force) : AddressField(s,defaultCS, force) {}
    ReplyTo(const QString &s, QFont::CharSet cs)         : AddressField(s, cs)       {}
    ~ReplyTo()                                                                       {}

    virtual const char* type()      { return "Reply-To"; }

};


/* Represents a "Organization" header */
class Organization : public Base {

  public:
    Organization()                                            {}
    Organization(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s,defaultCS,force); }
    Organization(const QString &s, QFont::CharSet cs)         { fromUnicodeString(s, cs); }
    ~Organization()                                           {}

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { o_rga.truncate(0); }
    virtual bool isEmpty()          { return (o_rga.isEmpty()); }
    virtual const char* type()      { return "Organization"; }

  protected:
    QString o_rga;

};


/* Represents a "Date" header */
class Date : public Base {

  public:
    Date()                  { t_ime=0; }
    Date(time_t t)          { t_ime=t; }
    Date(const QCString &s) { from7BitString(s, QFont::ISO_8859_1, false); }
    Date(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~Date()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
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


/* Represents a "To" header */
class To : public Base {

  public:
    To()                                             : a_ddrList(0) {}
    To(const QCString &s, QFont::CharSet defaultCS, bool force)  : a_ddrList(0) { from7BitString(s, defaultCS, force); }
    To(const QString &s, QFont::CharSet cs)          : a_ddrList(0) { fromUnicodeString(s, cs); }
    ~To()                                                           { delete a_ddrList; }

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { delete a_ddrList; a_ddrList=0; }
    virtual bool isEmpty()          { return (!a_ddrList || a_ddrList->isEmpty()
                                              || a_ddrList->first()->isEmpty()); }
    virtual const char* type()      { return "To"; }

    void addAddress(const AddressField &a);
    void emails(QStrList *l);

  protected:
   AddressList *a_ddrList;

};


/* Represents a "CC" header */
class CC : public To {

  public:
    CC()                                                                {}
    CC(const QCString &s, QFont::CharSet defaultCS, bool force)   : To(s,defaultCS,force) {}
    CC(const QString &s, QFont::CharSet cs)           : To(s,cs)        {}
    ~CC()                                                               {}

    virtual const char* type()      { return "CC"; }

};


/* Represents a "BCC" header */
class BCC : public To {

  public:
    BCC()                                                                {}
    BCC(const QCString &s, QFont::CharSet defaultCS, bool force)   : To(s,defaultCS,force) {}
    BCC(const QString &s, QFont::CharSet cs)           : To(s,cs)        {}
    ~BCC()                                                               {}

    virtual const char* type()      { return "BCC"; }

};


/* Represents a "Newsgroups" header */
class Newsgroups : public Base {

  public:
    Newsgroups()                  {}
    Newsgroups(const QCString &s) { from7BitString(s, QFont::ISO_8859_1, false); }
    Newsgroups(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~Newsgroups()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { g_roups.resize(0); }
    virtual bool isEmpty()          { return g_roups.isEmpty(); }
    virtual const char* type()      { return "Newsgroups"; }

    QCString firstGroup();

  protected:
    QCString g_roups;

};


/* Represents a "Followup-To" header */
class FollowUpTo : public Newsgroups {

  public:
    FollowUpTo()                                   {}
    FollowUpTo(const QCString &s)  : Newsgroups(s) {}
    FollowUpTo(const QString &s)   : Newsgroups(s) {}
    ~FollowUpTo()                                  {}

    virtual const char* type()        { return "Followup-To"; }

};


/* Represents a "Lines" header */
class Lines : public Base {

  public:
    Lines()                  : l_ines(-1) {}
    Lines(unsigned int i)    : l_ines(i)  {}
    Lines(const QCString &s) { from7BitString(s, QFont::ISO_8859_1, false); }
    Lines(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~Lines()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool );
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { l_ines=-1; }
    virtual bool isEmpty()          { return (l_ines==-1); }
    virtual const char* type()      { return "Lines"; }

    int numberOfLines()             { return l_ines; }
    void setNumberOfLines(int i)    { l_ines=i; }

  protected:
    int l_ines;

};


/* Represents a "References" header */
class References : public Base {

  public:
    References()                  : pos(-1) {}
    References(const QCString &s) { from7BitString(s, QFont::ISO_8859_1,false); }
    References(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~References()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { r_ef.resize(0); pos=0; }
    virtual bool isEmpty()          { return (r_ef.isEmpty()); }
    virtual const char* type()      { return "References"; }

    int count();
    QCString first();
    QCString next();
    QCString at(unsigned int i);

  protected:
    QCString r_ef;
    int pos;

};


/* Represents a "User-Agent" header */
class UserAgent : public Base {

  public:
    UserAgent()                  {}
    UserAgent(const QCString &s) { from7BitString(s, QFont::ISO_8859_1, false); }
    UserAgent(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~UserAgent()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
    virtual QString asUnicodeString();
    virtual void clear()            { u_agent.resize(0); }
    virtual bool isEmpty()          { return (u_agent.isEmpty()); }
    virtual const char* type()      { return "User-Agent"; }

  protected:
    QCString u_agent;

};


/* Represents a "Content-Type" header */
class ContentType : public Base {

  public:
    ContentType()                                            : m_imeType("text/plain"), c_ategory(CCsingle), f_orceDefaultCS(false) {}
    ContentType(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s, defaultCS, force); }
    ContentType(const QString &s, QFont::CharSet defaultCS, bool force)  { f_orceDefaultCS=force; fromUnicodeString(s, defaultCS); }
    ~ContentType()                                           {}

    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { m_imeType.resize(0); p_arams.resize(0); }
    virtual bool isEmpty()          { return (m_imeType.isEmpty()); }
    virtual const char* type()      { return "Content-Type"; }

    // overide the specified charset
    void setForceDefaultCS(bool b)  { f_orceDefaultCS=b; }

    //mime-type handling
    QCString mimeType()                     { return m_imeType; }
    QCString mediaType();
    QCString subType();
    void setMimeType(const QCString &s);
    bool isMediatype(const char *s);
    bool isSubtype(const char *s);
    bool isText();
    bool isPlainText();
    bool isHTMLText();
    bool isImage();
    bool isMultipart();
    bool isPartial();

    //parameter handling
    QCString charset();
    void setCharset(const QCString &s);
    QCString boundary();
    void setBoundary(const QCString &s);
    QString name();
    void setName(const QString &s, QFont::CharSet cs);
    QCString id();
    void setId(const QCString &s);
    int partialNumber();
    int partialCount();
    void setPartialParams(int total, int number);

    //category
    contentCategory category()            { return c_ategory; }
    void setCategory(contentCategory c)   { c_ategory=c; }

  protected:
    QCString getParameter(const char *name);
    void setParameter(const QCString &name, const QCString &value, bool doubleQuotes=false);
    QCString m_imeType, p_arams;
    contentCategory c_ategory;
    bool f_orceDefaultCS;

};


/* Represents a "Content-Transfer-Encoding" header */
class CTEncoding : public Base {

  public:
    CTEncoding()                  : c_te(CE7Bit), d_ecoded(true) {}
    CTEncoding(const QCString &s) { from7BitString(s, QFont::ISO_8859_1, false); }
    CTEncoding(const QString &s)  { fromUnicodeString(s, QFont::ISO_8859_1); }
    ~CTEncoding()                 {}

    virtual void from7BitString(const QCString &s, QFont::CharSet, bool);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
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


/* Represents a "Content-Disposition" header */
class CDisposition : public Base {

  public:
    CDisposition()                                            : d_isp(CDinline) {}
    CDisposition(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s, defaultCS, force); }
    CDisposition(const QString &s, QFont::CharSet cs)         { fromUnicodeString(s, cs); }
    ~CDisposition()                                           {}

    virtual void from7BitString(const QCString &s,QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet);
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


/* Represents a "Content-Description" header */
class CDescription : public Base {

  public:
    CDescription()                                            {}
    CDescription(const QCString &s, QFont::CharSet defaultCS, bool force) { from7BitString(s, defaultCS, force); }
    CDescription(const QString &s, QFont::CharSet cs)         { fromUnicodeString(s, cs); }
    ~CDescription()                                           {}
    
    virtual void from7BitString(const QCString &s, QFont::CharSet defaultCS, bool force);
    virtual QCString as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, QFont::CharSet cs);
    virtual QString asUnicodeString();
    virtual void clear()            { d_esc.truncate(0); }
    virtual bool isEmpty()          { return (d_esc.isEmpty()); }
    virtual const char* type()      { return "Content-Description"; }

  protected:
    QString d_esc;

};


};  //namespace KNHeaders


#endif // KNHEADERS_H

