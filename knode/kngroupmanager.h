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

#ifndef KNGROUPMANAGER_H
#define KNGROUPMANAGER_H

#include "kngroup.h"
#include "knjobdata.h"
#include "knnntpaccount.h"

#include <QObject>
#include <qlist.h>

class KNArticleManager;
class KNCleanUp;


//=================================================================================

/** Helper classes for the group selection dialog,
    contains info about a newsgroup (name, description) */

class KNGroupInfo {

  public:

    KNGroupInfo();
    KNGroupInfo(const QString &n_ame, const QString &d_escription, bool n_ewGroup=false, bool s_ubscribed=false, KNGroup::Status s_tatus=KNGroup::unknown );
    ~KNGroupInfo();

    /** group names will be utf-8 encoded in the future... */
    QString name,description;
    bool newGroup, subscribed;
    KNGroup::Status status;

    bool operator== (const KNGroupInfo &gi2) const;
    bool operator< (const KNGroupInfo &gi2) const;
};


/** Data of group list jobs. */
class KNGroupListData : public KNJobItem {

  public:
    /**
      Shared pointer to a KNGroupListData. To be used instead of raw KNGroupListData*.
    */
    typedef boost::shared_ptr<KNGroupListData> Ptr;

    KNGroupListData();
    ~KNGroupListData();

    bool readIn(KNJobData *job=0);
    bool writeOut();
    void merge(QList<KNGroupInfo>* newGroups);

    QList<KNGroupInfo>* extractList();

    QStringList subscribed;
    QString path;
    QList<KNGroupInfo> *groups;
    QDate fetchSince;
    bool getDescriptions;
    QTextCodec *codecForDescriptions;

};

//===============================================================================


/** Group manager. */
class KNGroupManager : public QObject , public KNJobConsumer {

  Q_OBJECT

  public:

    explicit KNGroupManager( QObject * parent = 0 );
    ~KNGroupManager();

    // group access
    void loadGroups( KNNntpAccount::Ptr a );
    void getSubscribed( KNNntpAccount::Ptr a, QStringList &l );
    /**
     * Returns the list of (subscribed) groups in the account @p a.
     */
    KNGroup::List groupsOfAccount( KNNntpAccount::Ptr a );

    bool loadHeaders( KNGroup::Ptr g );
    bool unloadHeaders( KNGroup::Ptr g, bool force = true );

    /**
     * Returns a group named @p gName in the server @p s, or null if none is found.
     */
    KNGroup::Ptr group( const QString &gName, const KNServerInfo::Ptr s );
    /**
     * Returns the first group in the server @p s, or null if it is empty.
     */
    KNGroup::Ptr firstGroupOfAccount( const KNServerInfo::Ptr s );
    KNGroup::Ptr currentGroup() const { return c_urrentGroup; }
    bool hasCurrentGroup() const               { return (c_urrentGroup!=0); }
    void setCurrentGroup( KNGroup::Ptr g );

    // group handling
    void showGroupDialog( KNNntpAccount::Ptr a, QWidget *parent = 0 );
    void subscribeGroup( const KNGroupInfo *gi, KNNntpAccount::Ptr a );
    bool unsubscribeGroup( KNGroup::Ptr g = KNGroup::Ptr() );
    /**
     * Shows the property dialog of @p g or if null, the properties of the currentGroup().
     */
    void showGroupProperties( KNGroup::Ptr g = KNGroup::Ptr() );
    void expireGroupNow( KNGroup::Ptr g = KNGroup::Ptr() );
    void reorganizeGroup( KNGroup::Ptr g = KNGroup::Ptr() );

    void checkGroupForNewHeaders( KNGroup::Ptr g = KNGroup::Ptr() );
    void checkAll( KNNntpAccount::Ptr a, bool silent = false );
    /**
     * Convenient method to call checkAll(KNNntpAccount::Ptr,bool) with the account
     * whose id is @p id.
     */
    void checkAll( int id, bool silent = false );

    void expireAll(KNCleanUp *cup);
    void expireAll( KNNntpAccount::Ptr a );
    void syncGroups();

  public slots:
    /** load group list from disk (if this fails: ask user if we should fetch the list) */
    void slotLoadGroupList( KNNntpAccount::Ptr a );
    /** fetch group list from server */
    void slotFetchGroupList( KNNntpAccount::Ptr a );
    /** check for new groups (created after the given date) */
    void slotCheckForNewGroups( KNNntpAccount::Ptr a, QDate date );

  protected:
    /** Reimplemented from KNJobConsumer */
    void processJob(KNJobData *j);
    KNGroup::List mGroupList;
    KNGroup::Ptr c_urrentGroup;
    KNArticleManager *a_rticleMgr;

  signals:
    void newListReady( KNGroupListData::Ptr d );

    /**
     * Emitted when a group is added.
     */
    void groupAdded( KNGroup::Ptr g );
    /**
     * Emitted when a group is removed.
     */
    void groupRemoved( KNGroup::Ptr g );
    /**
     * Emitted when a group is updated.
     */
    void groupUpdated( KNGroup::Ptr g );

};



#endif
