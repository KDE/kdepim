/*
    knnntpaccount.h

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

#ifndef KNNNTPACCOUNT_H
#define KNNNTPACCOUNT_H

#include <qdatetime.h>

#include "kncollection.h"
#include "knserverinfo.h"

namespace KNConfig {
class Identity;
};


class KNNntpAccount : public KNCollection , public KNServerInfo {
  
  public:
    KNNntpAccount();
    ~KNNntpAccount();
    
    collectionType type()             { return CTnntpAccount; }   
                  
    /** trys to read information, returns false if it fails to do so */
    bool readInfo(const QString &confPath);
    void saveInfo();    
    //void syncInfo();
    QString path();
    /** returns true when the user accepted */
    bool editProperties(QWidget *parent);
    
    //get
    bool fetchDescriptions()          { return f_etchDescriptions; }
    QDate lastNewFetch()              { return l_astNewFetch; }
    bool wasOpen()                    { return w_asOpen; }
    bool useDiskCache()               { return u_seDiskCache; }
    KNConfig::Identity* identity()    { return i_dentity; }

    //set
    void setFetchDescriptions(bool b) { f_etchDescriptions = b; }
    void setLastNewFetch(QDate date)  { l_astNewFetch = date; }
    void setUseDiskCache(bool b)      { u_seDiskCache=b; }
  
  protected:
    /** server specific identity */
    KNConfig::Identity *i_dentity;
    /** use an additional "list newsgroups" command to fetch the newsgroup descriptions */
    bool f_etchDescriptions;
    /** last use of "newgroups" */
    QDate l_astNewFetch;
    /** was the server open in the listview on the last shutdown? */
    bool w_asOpen;
    /** cache fetched articles on disk */
    bool u_seDiskCache;
};

#endif
