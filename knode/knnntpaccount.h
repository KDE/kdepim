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

#ifndef KNNNTPACCOUNT_H
#define KNNNTPACCOUNT_H

#include "configuration/settings_container_interface.h"
#include "kncollection.h"
#include "knserverinfo.h"

#include <QObject>
#include <QDate>

class QTimer;
class KNNntpAccount;
namespace KNode {
  class Cleanup;
}
namespace KPIMIdentities {
  class Identity;
}

/** Handles the interval checking of an news server account. */
class KNNntpAccountIntervalChecking : public QObject  {

  Q_OBJECT

  public:
    explicit KNNntpAccountIntervalChecking(KNNntpAccount *account);
    ~KNNntpAccountIntervalChecking();
    void installTimer();
    void deinstallTimer();

  protected:
    QTimer *t_imer;
    KNNntpAccount *a_ccount;

  protected slots:
    void slotCheckNews();

};


/** Represents an account on a news server. */
class KNNntpAccount : public KNCollection , public KNServerInfo, public KNode::SettingsContainerInterface
{
  public:
    /**
     * Shared pointer to a KNNntpAccount. To be used instead of raw KNNntpAccount*.
     */
    typedef boost::shared_ptr<KNNntpAccount> Ptr;
    /**
     * List of accounts.
     */
    typedef QList<KNNntpAccount::Ptr> List;

    KNNntpAccount();
    ~KNNntpAccount();

    collectionType type()             { return CTnntpAccount; }

    /** tries to read information, returns false if it fails to do so */
    bool readInfo(const QString &confPath);
    void writeConfig();
    //void syncInfo();
    QString path();
    /** returns true when the user accepted */
    bool editProperties(QWidget *parent);

    // news interval checking
    void startTimer();

    //get
    bool fetchDescriptions() const         { return f_etchDescriptions; }
    QDate lastNewFetch() const             { return l_astNewFetch; }
    bool wasOpen() const                   { return w_asOpen; }
    bool useDiskCache() const              { return u_seDiskCache; }

    /**
      Returns this server's specific identity or
      the null identity if there is none.
    */
    virtual const KPIMIdentities::Identity & identity() const;
    /**
      Sets this server's specific identity
      @param identity this server's identity of a null identity to unset.
    */
    virtual void setIdentity( const KPIMIdentities::Identity &identity );

    bool intervalChecking() const          { return i_ntervalChecking; }
    int checkInterval() const              { return c_heckInterval; }
    KNode::Cleanup *cleanupConfig() const { return mCleanupConf; }

    /** Returns the cleanup configuration that should be used for this account */
    KNode::Cleanup *activeCleanupConfig() const;

    //set
    void setFetchDescriptions(bool b) { f_etchDescriptions = b; }
    void setLastNewFetch(QDate date)  { l_astNewFetch = date; }
    void setUseDiskCache(bool b)      { u_seDiskCache=b; }
    void setCheckInterval(int c);
    void setIntervalChecking(bool b)  { i_ntervalChecking=b; }

  protected:
    /**
      Unique object identifier of the identity of this server.
      -1 means there is no specific identity for this group
      (because KPIMIdentities::Identity::uoid() returns an unsigned int.
    */
    int mIdentityUoid;
    /** account specific cleanup configuration */
    KNode::Cleanup *mCleanupConf;
    /** use an additional "list newsgroups" command to fetch the newsgroup descriptions */
    bool f_etchDescriptions;
    /** last use of "newgroups" */
    QDate l_astNewFetch;
    /** was the server open in the listview on the last shutdown? */
    bool w_asOpen;
    /** cache fetched articles on disk */
    bool u_seDiskCache;
    /** is interval checking enabled */
    bool i_ntervalChecking;
    int c_heckInterval;

    /** helper class for news interval checking, manages the QTimer */
    KNNntpAccountIntervalChecking *a_ccountIntervalChecking;

    /**
     * Reimplemented from KNArticleCollection::selfPtr().
     */
    virtual KNCollection::Ptr selfPtr();
};

#endif
