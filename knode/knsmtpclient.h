/***************************************************************************
                          knsmtpclient.h  -  description
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


#ifndef KNSMTPCLIENT_H
#define KNSMTPCLIENT_H

#include <knprotocolclient.h>


class KNSmtpClient : public KNProtocolClient  {

  Q_OBJECT

  public:
    KNSmtpClient(int NfdPipeIn, int NfdPipeOut, QObject *parent=0, const char *name=0);
    ~KNSmtpClient();
  
  protected:

    virtual void processJob();         // examines the job and calls the suitable handling method
  
    void doMail();
  
    virtual bool openConnection();     // connect, handshake

};

#endif
