/*
    knmime.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNMIME_H
#define KNMIME_H

#include <qlist.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfont.h>
#include <qcolor.h>
#include <qasciidict.h>

#include "knheaders.h"
#include "knjobdata.h"

class KNMimeContent;
class KNLoadHelper;
typedef QValueList<QCString> QCStringList;


/* Base class for messages in mime format
   It contains all the enums, static functions
   and parser-classes, that are needed for
   mime handling */

class KNMimeBase {

  public:

    //enums
    enum articleType    { ATmimeContent,
                          ATremote,
                          ATlocal };

    //encode and decode
    static QString decodeRFC2047String(const QCString &src, const char **usedCS, const QCString &defaultCS, bool forceCS);
    // addressHeader:   if this flag is true, all special characters like <,>,[,],... will be encoded, too.
    static QCString encodeRFC2047String(const QString &src, const char *charset, bool addressHeader=false);

    //charset cache
    static QStrIList c_harsetCache;
    static const char* cachedCharset(const QCString &name);

    //generation of message-ids and boundaries
    static QCString uniqueString();
    static QCString multiPartBoundary();

    //string handling
    static QCString extractHeader(const QCString &src, const char *name);
    static QCString CRLFtoLF(const QCString &s);
    static QCString CRLFtoLF(const char *s);
    static QCString LFtoCRLF(const QCString &s);
    static void stripCRLF(char *str);
    static void removeQuots(QCString &str);
    static void removeQuots(QString &str);

    /* Helper-class: splits a multipart-message into single
       parts as described in RFC 2046 */
    class MultiPartParser {

      public:
        MultiPartParser(const QCString &src, const QCString &boundary);
        ~MultiPartParser();

        bool parse();
        QCStringList parts()    { return p_arts; }
        QCString preamble()     { return p_reamble; }
        QCString epilouge()     { return e_pilouge; }

      protected:
        QCString s_rc, b_oundary, p_reamble, e_pilouge;
        QCStringList p_arts;

    };

    /* Helper-class: trys to extract the data from a possibly
       uuencoded message */
    class UUParser {

      public:
        UUParser(const QCString &src, const QCString &subject);
        ~UUParser();

        bool parse();
        bool isPartial()            { return (p_artNr>-1 && t_otalNr>-1); }
        int partialNumber()         { return p_artNr; }
        int partialCount()          { return t_otalNr; }
        bool hasTextPart()          { return (t_ext.length()>1); }
        QCString textPart()         { return t_ext; }
        QStrList binaryParts()       { return b_ins; }
        QStrList filenames()         { return f_ilenames; }
        QStrList mimeTypes()         { return m_imeTypes; }

      protected:
        QCString s_rc, t_ext, s_ubject;
        QStrList b_ins, f_ilenames, m_imeTypes;
        int p_artNr, t_otalNr;

    };

    /* This class stores boolean values in single bytes.
       It provides a similiar functionality as QBitArray
       but requires much less memory.
       We use it to store the flags of an article */
    class BoolFlags {

      public:
        BoolFlags()       { clear(); }
        ~BoolFlags()      {}

        void set(unsigned int i, bool b=true);
        bool get(unsigned int i);
        void clear()            { bits[0]=0; bits[1]=0; }
        unsigned char *data()   { return bits; }

      protected:
        unsigned char bits[2];  //space for 16 flags
    };

};


/* This class encapsulates a mime-encoded content.
   It parses the given data and creates a tree-like
   structure, that represents the structure of the
   message */

class KNMimeContent : public KNMimeBase {

  friend class KNAttachment;

  public:
    typedef QList<KNMimeContent> List;

    KNMimeContent();
    KNMimeContent(const QCString &h, const QCString &b);
    virtual ~KNMimeContent();

    //type
    virtual articleType type()      { return ATmimeContent; }

    //content handling
    bool hasContent()               { return ( !h_ead.isEmpty() && (!b_ody.isEmpty() || (c_ontents && !c_ontents->isEmpty())) ); }
    void setContent(QStrList *l);
    void setContent(const QCString &s);
    virtual void parse();
    virtual void assemble();
    virtual void clear();

    //header access
    QCString head()       { return h_ead; }
    virtual KNHeaders::Base* getHeaderByType(const char *type);
    virtual void setHeader(KNHeaders::Base *h);
    virtual bool removeHeader(const char *type);
    bool hasHeader(const char *type)                                  { return (getHeaderByType(type)!=0); }
    KNHeaders::ContentType* contentType(bool create=true)             { KNHeaders::ContentType *p=0; return getHeaderInstance(p, create); }
    KNHeaders::CTEncoding* contentTransferEncoding(bool create=true)  { KNHeaders::CTEncoding *p=0; return getHeaderInstance(p, create); }
    KNHeaders::CDisposition* contentDisposition(bool create=true)     { KNHeaders::CDisposition *p=0; return getHeaderInstance(p, create); }
    KNHeaders::CDescription* contentDescription(bool create=true)     { KNHeaders::CDescription *p=0; return getHeaderInstance(p, create); }

    //content access
    int size();
    int storageSize();
    int lineCount();
    QCString body()       { return b_ody; }
    QCString encodedContent(bool useCrLf=false);
    QByteArray decodedContent();
    void decodedText(QString &s, bool trimText=false);
    void decodedText(QStringList &s, bool trimText=false);
    void fromUnicodeString(const QString &s);

    KNMimeContent* textContent();
    void attachments(List *dst, bool incAlternatives=false);
    void addContent(KNMimeContent *c, bool prepend=false);
    void removeContent(KNMimeContent *c, bool del=false);
    void changeEncoding(KNHeaders::contentEncoding e);

    //saves the encoded content to the given textstream
    // scrambleFromLines: replace "\nFrom " with "\n>From ", this is
    // needed to avoid problem with mbox-files
    void toStream(QTextStream &ts, bool scrambleFromLines=false);

    // this charset is used for all headers and the body
    // if the charset is not declared explictly
    QCString defaultCharset()                  { return QCString(d_efaultCS); }
    void setDefaultCharset(const QCString &cs) { d_efaultCS = cachedCharset(cs); }

    // use the default charset even if a different charset is
    // declared in the article
    bool forceDefaultCS()         {  return f_orceDefaultCS; }

    // enables/disables the force mode, housekeeping.
    // works correctly only when the article is completly empty or
    // completly loaded
    virtual void setForceDefaultCS(bool b);


  protected:
    QCString rawHeader(const char *name);
    bool decodeText();
    template <class T> T* getHeaderInstance(T *ptr, bool create);

    QCString  h_ead,
              b_ody;
    List *c_ontents;
    KNHeaders::List *h_eaders;
    const char *d_efaultCS;
    bool f_orceDefaultCS;

};


class KNHdrViewItem; //forward declaration
class KNArticleCollection;

/* This class encapsulates a generic article. It provides
   all the usual headers of a RFC822-message. Further more
   it contains an unique id and can store a pointer to a
   QListViewItem. It is used as a base class for all visible
   articles. */

class KNArticle : public KNMimeContent, public KNJobItem {

  public:
    typedef QList<KNArticle> List;

    KNArticle(KNArticleCollection *c);
    ~KNArticle();

    //content handling
    void parse();
    void assemble();
    void clear();

    //header access
    KNHeaders::Base* getHeaderByType(const char *type);
    void setHeader(KNHeaders::Base *h);
    bool removeHeader(const char *type);
    virtual KNHeaders::MessageID* messageID(bool create=true)        { KNHeaders::MessageID *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Control* control(bool create=true)            { KNHeaders::Control *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Supersedes* supersedes(bool create=true)      { KNHeaders::Supersedes *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Subject* subject(bool create=true)            { if(!create && s_ubject.isEmpty()) return 0; return &s_ubject; }
    virtual KNHeaders::Date* date(bool create=true)                  { if(!create && d_ate.isEmpty()) return 0;return &d_ate; }
    virtual KNHeaders::From* from(bool create=true)                  { KNHeaders::From *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Organization* organization(bool create=true)  { KNHeaders::Organization *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::ReplyTo* replyTo(bool create=true)            { KNHeaders::ReplyTo *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::MailCopiesTo* mailCopiesTo(bool create=true)  { KNHeaders::MailCopiesTo *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::To* to(bool create=true)                      { KNHeaders::To *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::CC* cc(bool create=true)                      { KNHeaders::CC *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::BCC* bcc(bool create=true)                    { KNHeaders::BCC *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Newsgroups* newsgroups(bool create=true)      { KNHeaders::Newsgroups *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::FollowUpTo* followUpTo(bool create=true)      { KNHeaders::FollowUpTo *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::References* references(bool create=true)      { KNHeaders::References *p=0; return getHeaderInstance(p, create); }
    virtual KNHeaders::Lines* lines(bool create=true)                { if(!create && l_ines.isEmpty()) return 0; return &l_ines; }
    virtual KNHeaders::UserAgent* userAgent(bool create=true)        { KNHeaders::UserAgent *p=0; return getHeaderInstance(p, create); }

    //id
    int id()             { return i_d; }
    void setId(int i)    { i_d=i; }

    //list item handling
    KNHdrViewItem* listItem()            { return i_tem; }
    void setListItem(KNHdrViewItem *i);
    virtual void updateListItem() {}

    //network lock (reimplemented from KNJobItem)
    bool isLocked()                      { return f_lags.get(0); }
    void setLocked(bool b=true);

    //prevent that the article is unloaded automatically
    bool isNotUnloadable()               { return f_lags.get(1); }
    void setNotUnloadable(bool b=true)   { f_lags.set(1, b); }

    //article-collection
    KNArticleCollection* collection()           { return c_ol; }
    void setCollection(KNArticleCollection *c)  { c_ol=c; }
    bool isOrphant()                            { return (i_d==-1); }

    //charset-handling
    virtual void setForceDefaultCS(bool b);

  protected:
    //hardcoded headers
    KNHeaders::Subject s_ubject;
    KNHeaders::Date d_ate;
    KNHeaders::Lines l_ines;

    int i_d; //unique in the given collection
    KNArticleCollection *c_ol;
    KNHdrViewItem *i_tem;
    BoolFlags f_lags; // some status info

}; // KNArticle


class KNGroup;

/* KNRemoteArticle represents an article, whos body has to be
   retrieved from a remote host or from the local cache.
   All articles in a newsgroup are stored in instances
   of this class. */

class KNRemoteArticle : public KNArticle {

  public:
    typedef QList<KNRemoteArticle> List;

    KNRemoteArticle(KNGroup *g);
    ~KNRemoteArticle();

    // type
    articleType type() { return ATremote; }

    // content handling
    virtual void parse();
    virtual void assemble() {} //assembling is disabled for remote articles
    virtual void clear();

    // header access
    KNHeaders::Base* getHeaderByType(const char *type);
    void setHeader(KNHeaders::Base *h);
    bool removeHeader(const char *type);
    KNHeaders::MessageID* messageID(bool create=true)   { if(!create && m_essageID.isEmpty()) return 0; return &m_essageID; }
    KNHeaders::From* from(bool create=true)             { if(!create && f_rom.isEmpty()) return 0; return &f_rom; }
    KNHeaders::References* references(bool create=true) { if(!create && r_eferences.isEmpty()) return 0; return &r_eferences; }

    // article number
    int articleNumber()                  { return a_rticleNumber; }
    void setArticleNumber(int number)    { a_rticleNumber = number; }

    // status
    bool isNew()                         { return f_lags.get(2); }
    void setNew(bool b=true)             { f_lags.set(2, b); }
    bool isRead()                        { return f_lags.get(3); }
    void setRead(bool b=true)            { f_lags.set(3, b); }
    bool isExpired()                     { return f_lags.get(4); }
    void setExpired(bool b=true)         { f_lags.set(4, b); }
    bool isKept()                        { return f_lags.get(5); }
    void setKept(bool b=true)            { f_lags.set(5, b); }
    bool hasChanged()                    { return f_lags.get(6); }
    void setChanged(bool b=true)         { f_lags.set(6, b); }
    bool isIgnored()                     { return f_lags.get(7); }
    void setIgnored(bool b=true)         { f_lags.set(7, b); }
    bool isWatched()                     { return f_lags.get(8); }
    void setWatched(bool b=true)         { f_lags.set(8, b); }

    // thread info
    int idRef()                                     { return i_dRef; }
    void setIdRef(int i)                            { i_dRef=i; }
    KNRemoteArticle* displayedReference()           { return d_ref; }
    void setDisplayedReference(KNRemoteArticle *dr) { d_ref=dr; }
    bool threadMode()                             { return f_lags.get(9); }
    void setThreadMode(bool b=true)               { f_lags.set(9, b); }
    unsigned char threadingLevel()                { return t_hrLevel; }
    void setThreadingLevel(unsigned char l)       { t_hrLevel=l; }
    short score()                                 { return s_core; }
    void setScore(short s)                        { s_core=s; }
    unsigned short newFollowUps()                 { return n_ewFups; }
    bool hasNewFollowUps()                        { return (n_ewFups>0); }
    void setNewFollowUps(unsigned short s)        { n_ewFups=s; }
    void incNewFollowUps(unsigned short s=1)      { n_ewFups+=s; }
    void decNewFollowUps(unsigned short s=1)      { n_ewFups-=s; }
    unsigned short unreadFollowUps()              { return u_nreadFups; }
    bool hasUnreadFollowUps()                     { return (u_nreadFups>0); }
    void setUnreadFollowUps(unsigned short s)     { u_nreadFups=s; }
    void incUnreadFollowUps(unsigned short s=1)   { u_nreadFups+=s; }
    void decUnreadFollowUps(unsigned short s=1)   { u_nreadFups-=s; }
    void thread(List &f);

    //filtering
    bool filterResult()                     { return f_lags.get(10); }
    void setFilterResult(bool b=true)       { f_lags.set(10, b); }
    bool isFiltered()                       { return f_lags.get(11); }
    void setFiltered(bool b=true)           { f_lags.set(11, b); }
    bool hasVisibleFollowUps()              { return f_lags.get(12); }
    void setVisibleFollowUps(bool b=true)   { f_lags.set(12, b); }

    // list item handling
    void initListItem();
    void updateListItem();

    void setForceDefaultCS(bool b);

    QColor color() const { return c_olor; }
    void setColor(const QColor& c) { c_olor = c; }

  protected:
    // hardcoded headers
    KNHeaders::MessageID m_essageID;
    KNHeaders::From f_rom;
    KNHeaders::References r_eferences;

    int a_rticleNumber;
    int i_dRef;                      // id of a reference-article (0 == none)
    KNRemoteArticle *d_ref;          // displayed reference-article (may differ from i_dRef)
    unsigned char t_hrLevel;         // quality of threading
    short s_core;                    // guess what ;-)
    QColor c_olor;                   // color for the header list
    unsigned short u_nreadFups,      // number of the article's unread follow-ups
                   n_ewFups;         // number of the article's new follow-ups

}; // KNRemoteArticle



/* This class encapsulates an article, that is
   stored locally in an MBOX-file. All own and
   saved articles are represented by instances
   of this class. */


class KNLocalArticle : public KNArticle {

  public:
    typedef QList<KNLocalArticle> List;

    KNLocalArticle(KNArticleCollection *c=0);
    ~KNLocalArticle();

    //type
    articleType type()      { return ATlocal; }

    //content handling
    void parse();
    void clear();

    // header access
    KNHeaders::Base* getHeaderByType(const char *type);
    void setHeader(KNHeaders::Base *h);
    bool removeHeader(const char *type);
    KNHeaders::Newsgroups* newsgroups(bool create=true)     { if ( (!create && n_ewsgroups.isEmpty()) ||
                                                                   (!create && !isSavedRemoteArticle() && !doPost()) )
                                                                return 0;
                                                              return &n_ewsgroups; }
    KNHeaders::To* to(bool create=true)                     { if ( (!create && t_o.isEmpty()) ||
                                                                   (!create && !isSavedRemoteArticle() && !doMail()) )
                                                                return 0;
                                                              return &t_o; }

    //send article as mail
    bool doMail()                 { return f_lags.get(2); }
    void setDoMail(bool b=true)   { f_lags.set(2, b); }
    bool mailed()                 { return f_lags.get(3); }
    void setMailed(bool b=true)   { f_lags.set(3, b); }

    //post article to a newsgroup
    bool doPost()                 { return f_lags.get(4); }
    void setDoPost(bool b=true)   { f_lags.set(4, b); }
    bool posted()                 { return f_lags.get(5); }
    void setPosted(bool b=true)   { f_lags.set(5, b); }
    bool canceled()               { return f_lags.get(6); }
    void setCanceled(bool b=true) { f_lags.set(6, b); }

    // status
    bool pending()                { return ( (doPost() && !posted()) || (doMail() && !mailed()) ); }
    bool isSavedRemoteArticle()   {  return ( !doPost() && !doMail() && editDisabled() ); }

    //edit
    bool editDisabled()               { return f_lags.get(7); }
    void setEditDisabled(bool b=true) { f_lags.set(7, b); }

    //MBOX infos
    int startOffset()             { return s_Offset; }
    void setStartOffset(int so)   { s_Offset=so; }
    int endOffset()               { return e_Offset; }
    void setEndOffset(int eo)     { e_Offset=eo; }

    //nntp-server id
    int serverId()                { if(!doPost()) return -1; else return s_erverId; }
    void setServerId(int i)       { s_erverId=i; }

    //list item handling
    void updateListItem();

    void setForceDefaultCS(bool b);

    protected:
      //hardcoded headers
      KNHeaders::Newsgroups n_ewsgroups;
      KNHeaders::To t_o;
      int s_Offset, //position in mbox-file : start
          e_Offset, //position in mbox-file : end
          s_erverId; //id of the nntp-server this article is posted to
};


/* KNAttachment represents a file that is
   or will be attached to an article. */

class KNAttachment {

  public:
    KNAttachment(KNMimeContent *c);
    KNAttachment(KNLoadHelper *helper);
    ~KNAttachment();

    //name (used as a Content-Type parameter and as filename)
    const QString& name()           { return n_ame; }
    void setName(const QString &s)  { n_ame=s; h_asChanged=true; }

    //mime type
    const QCString& mimeType()            { return m_imeType; }
    void setMimeType(const QString &s);

    //Content-Description
    const QString& description()          { return d_escription; }
    void setDescription(const QString &s) { d_escription=s; h_asChanged=true; }

    //Encoding
    int cte()                             { return e_ncoding.cte(); }
    void setCte(int e)                    { e_ncoding.setCte( (KNHeaders::contentEncoding)(e) );
                                            h_asChanged=true; }
    bool isFixedBase64()                  { return f_b64; }
    QString encoding()                    { return e_ncoding.asUnicodeString(); }

    //content handling
    KNMimeContent* content()              { return c_ontent; }
    QString contentSize();
    bool isAttached()                     { return i_sAttached; }
    bool hasChanged()                     { return h_asChanged; }
    void updateContentInfo();
    void attach(KNMimeContent *c);
    void detach(KNMimeContent *c);

  protected:
    KNMimeContent *c_ontent;
    KNLoadHelper *l_oadHelper;
    QFile *f_ile;
    QCString m_imeType;
    QString n_ame,
            d_escription;
    KNHeaders::CTEncoding e_ncoding;
    bool  i_sAttached,
          h_asChanged,
          f_b64;
};

// some compilers (for instance Compaq C++) need template inline functions
// here rather than in the *.cpp file

template <class T> T* KNMimeContent::getHeaderInstance(T *ptr, bool create)
{
  T dummy; //needed to access virtual member T::type()

  ptr=static_cast <T*> (getHeaderByType(dummy.type()));
  if(!ptr && create) { //no such header found, but we need one => create it
    ptr=new T(this);
    if(!(h_eaders)) {
      h_eaders=new KNHeaders::List();
      h_eaders->setAutoDelete(true);
    }
    h_eaders->append(ptr);
  }

  return ptr;
}

#endif //KNMIME_H
