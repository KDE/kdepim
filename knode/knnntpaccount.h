/***************************************************************************
                          knnntpaccount.h  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KNNNTPACCOUNT_H
#define KNNNTPACCOUNT_H

#include <qdatetime.h>

#include "kncollection.h"
#include "knserverinfo.h"


class KNNntpAccount : public KNCollection, public KNServerInfo {
  
  public:
    KNNntpAccount();
    ~KNNntpAccount();
    
    collectionType type()             { return CTnntpAccount; }   
                  
    // trys to read information, returns false if it fails to do so
    bool readInfo(const QString &confPath);
    void saveInfo();    
    void syncInfo();
    QString path();
    
    //get
    int unsentCount()                 { return u_nsentCount; }
    bool hasUnsent()                  { return (u_nsentCount>0); }
    bool fetchDescriptions()          { return f_etchDescriptions; }
    QDate lastNewFetch()              { return l_astNewFetch; }
  
    //set
    void setUnsentCount(int i)        { u_nsentCount=i; }
    void incUnsentCount(int i=1)      { u_nsentCount+=i; }
    void decUnsentCount(int i=1)      { u_nsentCount-=i; }
    void setFetchDescriptions(bool b) { f_etchDescriptions = b; }
    void setLastNewFetch(QDate date)  { l_astNewFetch = date; }
  
  protected:
    int u_nsentCount;
    bool f_etchDescriptions;          // use an additional "list newsgroups" command to fetch the newsgroup descriptions
    QDate l_astNewFetch;              // last use of "newgroups"
      
};

#endif
