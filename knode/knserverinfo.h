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

#ifndef KNSERVERINFO_H
#define KNSERVERINFO_H

#include <qstring.h>

class KConfig;
namespace KWallet {
  class Wallet;
}
using KWallet::Wallet;

class KNServerInfo {

  public:
    enum serverType { STnntp, STsmtp, STpop3 };
    enum Encryption { None, SSL, TLS };

    KNServerInfo();
    ~KNServerInfo();

    void readConf(KConfig *conf);
    void saveConf(KConfig *conf);

    //get
    serverType type()const         { return t_ype; }
    int id()const                  { return i_d; }
    const QString& server()   { return s_erver; }
    const QString& user()     { return u_ser; }
    const QString& pass();
    int port() const                { return p_ort; }
    int hold() const               { return h_old; }
    int timeout() const            { return t_imeout; }
    bool needsLogon()const         { return n_eedsLogon; }
    bool isEmpty()const            { return s_erver.isEmpty(); }
    bool readyForLogin() const { return !n_eedsLogon || mPassLoaded; }
    Encryption encryption() const { return mEncryption; }

    //set
    void setType(serverType t)        { t_ype=t; }
    void setId(int i)                 { i_d=i; }
    void setServer(const QString &s)  { s_erver=s; }
    void setUser(const QString &s)    { u_ser=s; }
    void setPass(const QString &s);
    void setPort(int p)               { p_ort=p; }
    void setHold(int h)               { h_old=h; }
    void setTimeout(int t)            { t_imeout=t; }
    void setNeedsLogon(bool b)        { n_eedsLogon=b; }
    void setEncryption( Encryption enc ) { mEncryption = enc; }

    bool operator==(const KNServerInfo &s);

    /** Loads the password from KWallet, used for on-demand password loading */
    void readPassword();

  protected:
    serverType t_ype;

    QString  s_erver,
             u_ser,
             p_ass;

    int i_d,
        p_ort,
        h_old,
        t_imeout;

    bool n_eedsLogon,
         p_assDirty;
    /** Prevent loading the password multiple times since wallet operations
        from the I/O thread don't work. */
    bool mPassLoaded;
    /** Encyrption method */
    Encryption mEncryption;
};


#endif
