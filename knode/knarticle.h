/*
    knarticle.h

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

#ifndef KNARTICLE_H
#define KNARTICLE_H

#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfont.h>
#include <qcolor.h>
#include <qasciidict.h>
#include <qptrlist.h>

#include <kmime_headers.h>
#include <kmime_newsarticle.h>
#include <boolflags.h>

#include "knjobdata.h"

//forward declarations
class KNLoadHelper;
class KNHdrViewItem;
class KNArticleCollection;

/** This class encapsulates a generic article. It provides all the
    usual headers of a RFC822-message. Further more it contains an
    unique id and can store a pointer to a @ref QListViewItem. It is
    used as a base class for all visible articles. */

class KNArticle : public KMime::NewsArticle, public KNJobItem {

  public:
    typedef QPtrList<KNArticle> List;

    KNArticle(KNArticleCollection *c);
    ~KNArticle();

    //id
    int id() const            { return i_d; }
    void setId(int i)    { i_d=i; }

    //list item handling
    KNHdrViewItem* listItem() const           { return i_tem; }
    void setListItem(KNHdrViewItem *i);
    virtual void updateListItem() {}

    //network lock (reimplemented from KNJobItem)
    bool isLocked()                      { return f_lags.get(0); }
    void setLocked(bool b=true);

    //prevent that the article is unloaded automatically
    bool isNotUnloadable()               { return f_lags.get(1); }
    void setNotUnloadable(bool b=true)   { f_lags.set(1, b); }

    //article-collection
    KNArticleCollection* collection() const          { return c_ol; }
    void setCollection(KNArticleCollection *c)  { c_ol=c; }
    bool isOrphant() const                           { return (i_d==-1); }

  protected:
    int i_d; //unique in the given collection
    KNArticleCollection *c_ol;
    KNHdrViewItem *i_tem;

}; // KNArticle


class KNGroup;

/** KNRemoteArticle represents an article, whos body has to be
    retrieved from a remote host or from the local cache.
    All articles in a newsgroup are stored in instances
    of this class. */

class KNRemoteArticle : public KNArticle {

  public:
    typedef QPtrList<KNRemoteArticle> List;

    KNRemoteArticle(KNGroup *g);
    ~KNRemoteArticle();

    // type
    articleType type() { return ATremote; }

    // content handling
    virtual void parse();
    virtual void assemble() {} //assembling is disabled for remote articles
    virtual void clear();

    // header access
    KMime::Headers::Base* getHeaderByType(const char *type);
    void setHeader(KMime::Headers::Base *h);
    bool removeHeader(const char *type);
    KMime::Headers::MessageID* messageID(bool create=true)   { if(!create && m_essageID.isEmpty()) return 0; return &m_essageID; }
    KMime::Headers::From* from(bool create=true)             { if(!create && f_rom.isEmpty()) return 0; return &f_rom; }
    KMime::Headers::References* references(bool create=true) { if(!create && r_eferences.isEmpty()) return 0; return &r_eferences; }

    // article number
    int articleNumber() const                 { return a_rticleNumber; }
    void setArticleNumber(int number)    { a_rticleNumber = number; }

    // status
    bool isNew()                         { return f_lags.get(2); }
    void setNew(bool b=true)             { f_lags.set(2, b); }
    bool getReadFlag()                   { return f_lags.get(3); }
    bool isRead()                        { return f_lags.get(7) || f_lags.get(3); }   // ignored articles == read
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
    void setIdRef(int i)                            { if (i != id())
                                                        i_dRef=i;
                                                      else
                                                        i_dRef=0; }
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
    void setPgpSigned(bool f) { pgp_signed = f; }
    bool isPgpSigned() const { return pgp_signed; }

    time_t subThreadChangeDate() { return s_ubThreadChangeDate; }
    void setSubThreadChangeDate(time_t date) { s_ubThreadChangeDate = date; }
    // propagate the change date to the root article
    void propagateThreadChangedDate();

  protected:
    // hardcoded headers
    KMime::Headers::MessageID m_essageID;
    KMime::Headers::From f_rom;
    KMime::Headers::References r_eferences;

    int a_rticleNumber;
    int i_dRef;                      // id of a reference-article (0 == none)
    KNRemoteArticle *d_ref;          // displayed reference-article (may differ from i_dRef)
    unsigned char t_hrLevel;         // quality of threading
    short s_core;                    // guess what ;-)
    QColor c_olor;                   // color for the header list
    bool pgp_signed;                 // true if the user asks to check the pgp signature
    unsigned short u_nreadFups,      // number of the article's unread follow-ups
                   n_ewFups;         // number of the article's new follow-ups
    time_t s_ubThreadChangeDate;     // the last time the sub-thread of this article changed
                                     // i.e. when the last article arrived...

}; // KNRemoteArticle



/* This class encapsulates an article, that is
   stored locally in an MBOX-file. All own and
   saved articles are represented by instances
   of this class. */


class KNLocalArticle : public KNArticle {

  public:
    typedef QPtrList<KNLocalArticle> List;

    KNLocalArticle(KNArticleCollection *c=0);
    ~KNLocalArticle();

    //type
    articleType type()      { return ATlocal; }

    //content handling
    void parse();
    void clear();

    // header access
    KMime::Headers::Base* getHeaderByType(const char *type);
    void setHeader(KMime::Headers::Base *h);
    bool removeHeader(const char *type);
    KMime::Headers::Newsgroups* newsgroups(bool create=true)     { if ( (!create && n_ewsgroups.isEmpty()) ||
                                                                   (!create && !isSavedRemoteArticle() && !doPost()) )
                                                                return 0;
                                                              return &n_ewsgroups; }
    KMime::Headers::To* to(bool create=true)                     { if ( (!create && t_o.isEmpty()) ||
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

    //search
    bool filterResult()                { return f_lags.get(8); }
    void setFilterResult(bool b=true)  { f_lags.set(8, b); }

    //MBOX infos
    int startOffset() const             { return s_Offset; }
    void setStartOffset(int so)   { s_Offset=so; }
    int endOffset() const              { return e_Offset; }
    void setEndOffset(int eo)     { e_Offset=eo; }

    //nntp-server id
    int serverId()                { if(!doPost()) return -1; else return s_erverId; }
    void setServerId(int i)       { s_erverId=i; }

    //list item handling
    void updateListItem();

    void setForceDefaultCS(bool b);

    protected:
      //hardcoded headers
      KMime::Headers::Newsgroups n_ewsgroups;
      KMime::Headers::To t_o;
      int s_Offset, //position in mbox-file : start
          e_Offset, //position in mbox-file : end
          s_erverId; //id of the nntp-server this article is posted to
};


/* KNAttachment represents a file that is
   or will be attached to an article. */

class KNAttachment {

  public:
    KNAttachment(KMime::Content *c);
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
    void setCte(int e)                    { e_ncoding.setCte( (KMime::Headers::contentEncoding)(e) );
                                            h_asChanged=true; }
    bool isFixedBase64()const                  { return f_b64; }
    QString encoding()                    { return e_ncoding.asUnicodeString(); }

    //content handling
    KMime::Content* content()const             { return c_ontent; }
    QString contentSize();
    bool isAttached() const                    { return i_sAttached; }
    bool hasChanged() const                    { return h_asChanged; }
    void updateContentInfo();
    void attach(KMime::Content *c);
    void detach(KMime::Content *c);

  protected:
    KMime::Content *c_ontent;
    KNLoadHelper   *l_oadHelper;
    QFile *f_ile;
    QCString m_imeType;
    QString n_ame,
            d_escription;
    KMime::Headers::CTEncoding e_ncoding;
    bool  i_sAttached,
          h_asChanged,
          f_b64;
};

#endif //KNARTICLE_H
