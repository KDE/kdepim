/***************************************************************************
                          knnntpclient.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNNNTPCLIENT_H
#define KNNNTPCLIENT_H

#include <pthread.h>

#include <knprotocolclient.h>


class KNNntpClient : public KNProtocolClient  {

  Q_OBJECT

  public:
    
    KNNntpClient(int NfdPipeIn, int NfdPipeOut, pthread_mutex_t *nntpMutex, QObject *parent=0, const char *name=0);
    ~KNNntpClient();
    
  protected:

    virtual void processJob();         // examines the job and calls the suitable handling method
  
    void doLoadGroups();
    void doFetchGroups();
    void doCheckNewGroups();
    void doFetchNewHeaders();
    void doFetchArticle();
    void doPostArticle();
  
    virtual bool openConnection();     // connect, handshake
    virtual bool sendCommand(const QCString &cmd, int &rep);  // authentication on demand
    virtual void handleErrors();

    pthread_mutex_t *mutex;
    
};

#endif
