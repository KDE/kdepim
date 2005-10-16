/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGROUP_H
#define KNGROUP_H

#include "knarticlecollection.h"
#include "knjobdata.h"
#include "knarticle.h"

#include <kio/job.h>

#include <QByteArray>
#include <QList>
//Added by qt3to4:
#include <Q3CString>

class KNProtocolClient;
class KNNntpAccount;

namespace KNode {
  class Identity;
  class Cleanup;
}


/** Representation of a news group. This class contains:
 * - Static information about a news group (eg. the name and account)
 * - Dynamic information about a news group (eg. article serial numbers)
 * - Group specific settings (eg. identities or cleanup settings)
 * - Load and store methods for the header list of this group
 */
class KNGroup : public KNArticleCollection , public KNJobItem  {

  public:
    /** The posting rights status of this group. */
    enum Status { unknown=0, readOnly=1, postingAllowed=2, moderated=3 };

    KNGroup(KNCollection *p=0);
    ~KNGroup();

    /** type */
    collectionType type()               { return CTgroup; }

    /** list-item handling */
    void updateListItem();

    /** info */
    QString path();
    bool readInfo(const QString &confPath);
    void saveInfo();

    /** name */
    bool hasName() const                         { return (!n_ame.isEmpty()); }
    const QString& name();
    const QString& groupname()              { return g_roupname; }
    void setGroupname(const QString &s)     { g_roupname=s; }
    /** Returns the group description. */
    QString description() const { return d_escription; }
    /** Sets the group description.
     * @param s The group description.
     */
    void setDescription( const QString &s ) { d_escription = s; }

    /** count + numbers */
    int newCount() const               { return n_ewCount; }
    void setNewCount(int i)       { n_ewCount=i; }
    void incNewCount(int i=1)     { n_ewCount+=i; }
    void decNewCount(int i=1)     { n_ewCount-=i; }
    int firstNewIndex() const          { return f_irstNew; }
    void setFirstNewIndex(int i)  { f_irstNew=i; }

    int lastFetchCount() const         { return l_astFetchCount; }
    void setLastFetchCount(int i) { l_astFetchCount=i; }

    int readCount()const               { return r_eadCount; }
    void setReadCount(int i)      { r_eadCount=i; }
    void incReadCount(int i=1)    { r_eadCount+=i; }
    void decReadCount(int i=1)    { r_eadCount-=i; }

    /** Returns the smallest available article serial number in this group. */
    int firstNr() const { return f_irstNr; }
    /** Sets the smallest available serial number.
     * @param i The serial number.
     */
    void setFirstNr( int i ) { f_irstNr = i; }
    /** Returns the largest used article serial number in this group. */
    int lastNr() const { return l_astNr; }
    /** Sets the largest used serial number.
     * @param i The serial number.
     */
    void setLastNr( int i ) { l_astNr = i; }
    /** Returns the maximal number of articles to download from the server. */
    int maxFetch() const { return m_axFetch; }
    /** Sets the maximal number of articles to download from the server.
     * @param i The maximal number of articles to download.
     */
    void setMaxFetch( int i ) { m_axFetch = i; }

    int statThrWithNew();
    int statThrWithUnread();

    /** article access */
    KNRemoteArticle* at(int i)          { return static_cast<KNRemoteArticle*> (KNArticleCollection::at(i)); }
    KNRemoteArticle* byId(int id)       { return static_cast<KNRemoteArticle*> (KNArticleCollection::byId(id)); }
    KNRemoteArticle* byMessageId(const Q3CString &mId)
                                        { return static_cast<KNRemoteArticle*> (KNArticleCollection::byMessageId(mId)); }

    /** Load the stored headers from disk. */
    bool loadHdrs();
    bool unloadHdrs(bool force=true);
    void insortNewHeaders( const KIO::UDSEntryList &list, KNProtocolClient *client = 0 );
    int saveStaticData(int cnt,bool ovr=false);
    void saveDynamicData(int cnt,bool ovr=false);
    void syncDynamicData();

    /** mark articles with this id as read when we later load the headers / fetch new articles */
    void appendXPostID(const QString &id);
    void processXPostBuffer(bool deleteAfterwards);

    /** article handling */
    void updateThreadInfo();
    void reorganize();
    void scoreArticles(bool onlynew=true);

    /** locking */
    bool isLocked()             { return l_ocked; }
    void setLocked(bool l)      { l_ocked=l; }

    QString prepareForExecution();

    /** charset-handling */
    const Q3CString defaultCharset()           { return d_efaultChSet; }
    void setDefaultCharset(const Q3CString &s) { d_efaultChSet=s; }
    bool useCharset()                         { return ( u_seCharset && !d_efaultChSet.isEmpty() ); }
    void setUseCharset(bool b)                { u_seCharset=b; }

    /** Returns the account this group belongs to. */
    KNNntpAccount* account();
    /** Returns the identity configured for this group (might be empty). */
    KNode::Identity* identity() const { return i_dentity; }
    /** Sets the identity for this group.
     * @param i The identity.
     */
    void setIdentity(KNode::Identity *i) { i_dentity = i; }
    /** Returns the posting rights of this group.
     */
    Status status() const { return s_tatus; }
    /** Sets the posting rights for this group.
     * @param s The new posting rights.
     */
    void setStatus(Status s) { s_tatus = s; }
    /** Shows the group properties dialog.
     * @see KNGroupPropDlg
     */
    void showProperties();

    /** Returns the cleanup configuration of this group (might be empty). */
    KNode::Cleanup *cleanupConfig() const { return mCleanupConf; }
    /** Returns the active cleanup configuration of this group, ie. the
     * "lowest" available cleanup configuration.
     */
    KNode::Cleanup *activeCleanupConfig();


  protected:
    void buildThreads(int cnt, KNProtocolClient *client=0);
    KNRemoteArticle* findReference(KNRemoteArticle *a);

    int       n_ewCount,
              l_astFetchCount,
              r_eadCount,
              i_gnoreCount,
              f_irstNr,
              l_astNr,
              m_axFetch,
              d_ynDataFormat,
              f_irstNew;

    Q3CString  d_efaultChSet;
    QString   g_roupname,
              d_escription;

    bool      l_ocked,
              u_seCharset;

    Status    s_tatus;

    QStringList c_rosspostIDBuffer;

    /** Optional headers provided by the XOVER command.
     *  These headers will be saved within the static data.
     */
    QList<QByteArray> mOptionalHeaders;

    KNode::Identity *i_dentity;
    KNode::Cleanup *mCleanupConf;

    class dynDataVer0 {

      public:
        dynDataVer0()     { id=-1; idRef=-1; read=0; thrLevel=0; score=50; }
        ~dynDataVer0()    {}
        void setData(KNRemoteArticle *a);
        void getData(KNRemoteArticle *a);

        int id;
        int idRef;
        bool read;
        short thrLevel, score;
    };

    class dynDataVer1 {

      public:
        dynDataVer1()     { id=-1; idRef=-1; read=0; thrLevel=0; score=0, ignoredWatched=0; }
        void setData(KNRemoteArticle *a);
        void getData(KNRemoteArticle *a);

        int id;
        int idRef;
        bool read;
        short thrLevel, score;
        char ignoredWatched;
    };

};

#endif

// kate: space-indent on; indent-width 2;
