/***************************************************************************
                          knserverinfo.h  -  description
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


#ifndef KNSERVERINFO_H
#define KNSERVERINFO_H

#include <qstring.h>

class KConfig;


class KNServerInfo {
  
  public:
    enum serverType { STnntp, STsmtp, STpop3 };
    KNServerInfo();
    ~KNServerInfo();
    
    void copy(KNServerInfo *i);
    void clear();
    
    void readConf(KConfig *conf);
    void saveConf(KConfig *conf);
            
    //get
    serverType type()         { return t_ype; }
    int id()                  { return i_d; }
    const QCString& server()  { return s_erver; }
    const QCString& user()    { return u_ser; }
    const QCString& pass()    { return p_ass; }
    int port()                { return p_ort; }
    int hold()                { return h_old; }
    int timeout()             { return t_imeout; }
    bool needsLogon()         { return n_eedsLogon; }
    bool isEmpty()            { return s_erver.isEmpty(); }
    bool isEqual(KNServerInfo *i);
    
    //set
    void setType(serverType t)        { t_ype=t; }
    void setId(int i)                 { i_d=i; }
    void setServer(const QCString &s) { s_erver=s.copy(); }
    void setUser(const QCString &s)   { u_ser=s.copy(); }
    void setPass(const QCString &s)   { p_ass=s.copy(); }
    void setPort(int p)               { p_ort=p; }
    void setHold(int h)               { h_old=h; }
    void setTimeout(int t)            { t_imeout=t; }
    void setNeedsLogon(bool b)        { n_eedsLogon=b; }

  protected:
    serverType t_ype;

    QCString  s_erver,
              u_ser,
              p_ass;

    int i_d,
        p_ort,
        h_old,
        t_imeout;

    bool n_eedsLogon;
    
};  
      

#endif
