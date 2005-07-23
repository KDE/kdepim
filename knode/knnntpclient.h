/*
    knnntpclient.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNNNTPCLIENT_H
#define KNNNTPCLIENT_H

#include <qmutex.h>

#include <knprotocolclient.h>


class KNNntpClient : public KNProtocolClient  {

  public:
    
    KNNntpClient(int NfdPipeIn, int NfdPipeOut, QMutex& nntpMutex);
    ~KNNntpClient();
    
  protected:

    /** examines the job and calls the suitable handling method */
    virtual void processJob();
  
    void doLoadGroups();
    void doFetchGroups();
    void doCheckNewGroups();
    void doFetchNewHeaders();
    void doFetchArticle();
    void doPostArticle();
    void doFetchSource();

    /** connect, handshake */
    virtual bool openConnection();
    /** authentication on demand */
    virtual bool sendCommand(const QCString &cmd, int &rep);
    virtual void handleErrors();
    bool switchToGroup(const QString &newGroup);

    QString currentGroup;
    QMutex& mutex;
    
};

#endif
