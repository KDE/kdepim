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

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif
#include <QString>

class KConfigGroup;
namespace KWallet {
  class Wallet;
}
using KWallet::Wallet;


/** Represents an account on a news server. */
class KNServerInfo {

  public:
    enum Encryption { None, SSL, TLS };

    /**
      Shared pointer to a KNServerInfo. To be used instead of raw KNServerInfo*.
    */
    typedef boost::shared_ptr<KNServerInfo> Ptr;

    KNServerInfo();
    ~KNServerInfo();

    void readConf(KConfigGroup &conf);
    void saveConf(KConfigGroup &conf);

    //get
    int id()const                  { return i_d; }
    const QString& server()   { return s_erver; }
    const QString& user()     { return u_ser; }
    const QString& pass();
    int port() const                { return p_ort; }
    bool needsLogon()const         { return n_eedsLogon; }
    bool isEmpty()const            { return s_erver.isEmpty(); }
    bool readyForLogin() const { return !n_eedsLogon || mPassLoaded; }
    Encryption encryption() const { return mEncryption; }

    //set
    void setId(int i)                 { i_d=i; }
    void setServer(const QString &s)  { s_erver=s; }
    void setUser(const QString &s)    { u_ser=s; }
    void setPass(const QString &s);
    void setPort(int p)               { p_ort=p; }
    void setNeedsLogon(bool b)        { n_eedsLogon=b; }
    void setEncryption( Encryption enc ) { mEncryption = enc; }

    bool operator==(const KNServerInfo &s) const;

    /** Loads the password from KWallet, used for on-demand password loading */
    void readPassword();

  protected:
    QString  s_erver,
             u_ser,
             p_ass;

    int i_d, p_ort;

    bool n_eedsLogon,
         p_assDirty;
    /** Prevent loading the password multiple times since wallet operations
        from the I/O thread don't work. */
    bool mPassLoaded;
    /** Encyrption method */
    Encryption mEncryption;
};


#endif
