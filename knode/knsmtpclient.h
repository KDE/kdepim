/*
    knsmtpclient.h

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
