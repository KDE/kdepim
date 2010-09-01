/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klar�vdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


#ifndef KPIM_LDAPCLIENT_H
#define KPIM_LDAPCLIENT_H


#include <tqobject.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <tqmemarray.h>
#include <tqguardedptr.h>
#include <tqtimer.h>

#include <kio/job.h>
#include <kabc/ldif.h>
#include <kconfig.h>

#include <kdepimmacros.h>

namespace KPIM {

class LdapClient;
typedef TQValueList<TQByteArray> LdapAttrValue;
typedef TQMap<TQString,LdapAttrValue > LdapAttrMap;

class LdapServer
{
  public:
    LdapServer()
    : mPort( 389 ),
      mTimeLimit(0),
      mSizeLimit(0),
      mVersion(2),
      mSecurity(Sec_None),
      mAuth( LdapServer::Anonymous )
    {}

    enum Security{ Sec_None, TLS, SSL };
    enum Auth{ Anonymous, Simple, SASL };
    TQString host() const { return mHost; }
    int port() const { return mPort; }
    const TQString &baseDN() const { return mBaseDN; }
    const TQString &user() const { return mUser; }
    const TQString &bindDN() const { return mBindDN; }
    const TQString &pwdBindDN() const { return mPwdBindDN; }
    int timeLimit() const { return mTimeLimit; }
    int sizeLimit() const { return mSizeLimit; }
    int version() const { return mVersion; }
    int security() const { return mSecurity; }
    int auth() const { return mAuth; }
    const TQString &mech() const { return mMech; }

    void setHost( const TQString &host ) { mHost = host; }
    void setPort( int port ) { mPort = port; }
    void setBaseDN( const TQString &baseDN ) {  mBaseDN = baseDN; }
    void setUser( const TQString &user ) { mUser = user; }
    void setBindDN( const TQString &bindDN ) {  mBindDN = bindDN; }
    void setPwdBindDN( const TQString &pwdBindDN ) {  mPwdBindDN = pwdBindDN; }
    void setTimeLimit( int timelimit ) { mTimeLimit = timelimit; }
    void setSizeLimit( int sizelimit ) { mSizeLimit = sizelimit; }
    void setVersion( int version ) { mVersion = version; }
    void setSecurity( int security ) { mSecurity = security; } //0-No, 1-TLS, 2-SSL - KDE4: add an enum to Lda
    void setAuth( int auth ) { mAuth = auth; } //0-Anonymous, 1-simple, 2-SASL - KDE4: add an enum to LdapCon
    void setMech( const TQString &mech ) { mMech = mech; }

  private:
    TQString mHost;
    int mPort;
    TQString mBaseDN;
    TQString mUser;
    TQString mBindDN;
    TQString mPwdBindDN;
    TQString mMech;
    int mTimeLimit, mSizeLimit, mVersion, mSecurity, mAuth;
};


/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class LdapObject
{
  public:
    LdapObject()
      : dn( TQString::null ), client( 0 ) {}
    explicit LdapObject( const TQString& _dn, LdapClient* _cl ) : dn( _dn ), client( _cl ) {}
    LdapObject( const LdapObject& that ) { assign( that ); }

    LdapObject& operator=( const LdapObject& that )
    {
      assign( that );
      return *this;
    }

    TQString toString() const;

    void clear();

    TQString dn;
    TQString objectClass;
    LdapAttrMap attrs;
    LdapClient* client;

  protected:
    void assign( const LdapObject& that );

  private:
    //class LdapObjectPrivate* d;
};

/**
  * This class is internal. Binary compatibility might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KDE_EXPORT LdapClient : public QObject
{
  Q_OBJECT

  public:
    LdapClient( int clientNumber, TQObject* parent = 0, const char* name = 0 );
    virtual ~LdapClient();

    /*! returns true if there is a query running */
    bool isActive() const { return mActive; }

    int clientNumber() const;
    int completionWeight() const;
    void setCompletionWeight( int );

    const LdapServer& server() { return mServer; }
    void setServer( const LdapServer &server ) { mServer = server; }
    /*! Return the attributes that should be
     * returned, or an empty list if
     * all attributes are wanted
     */
    TQStringList attrs() const { return mAttrs; }

  signals:
    /*! Emitted when the query is done */
    void done();

    /*! Emitted in case of error */
    void error( const TQString& );

    /*! Emitted once for each object returned
     * from the query
     */
    void result( const KPIM::LdapObject& );

  public slots: // why are those slots?
    /*! Set the attributes that should be
     * returned, or an empty list if
     * all attributes are wanted
     */
    void setAttrs( const TQStringList& attrs );

    void setScope( const TQString scope ) { mScope = scope; }

    /*!
     * Start the query with filter filter
     */
    void startQuery( const TQString& filter );

    /*!
     * Abort a running query
     */
    void cancelQuery();

  protected slots:
    void slotData( KIO::Job*, const TQByteArray &data );
    void slotInfoMessage( KIO::Job*, const TQString &info );
    void slotDone();

  protected:
    void startParseLDIF();
    void parseLDIF( const TQByteArray& data );
    void endParseLDIF();
    void finishCurrentObject();

    LdapServer mServer;
    TQString mScope;
    TQStringList mAttrs;

    TQGuardedPtr<KIO::SimpleJob> mJob;
    bool mActive;
    bool mReportObjectClass;

    LdapObject mCurrentObject;

  private:
    KABC::LDIF mLdif;
    int mClientNumber;
    int mCompletionWeight;

//    class LdapClientPrivate;
//    LdapClientPrivate* d;
};

/**
 * Structure describing one result returned by a LDAP query
 */
struct LdapResult {
  TQString name;     ///< full name
  TQStringList email;    ///< emails
  int clientNumber; ///< for sorting in a ldap-only lookup
  int completionWeight; ///< for sorting in a completion list
};
typedef TQValueList<LdapResult> LdapResultList;


/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KDE_EXPORT LdapSearch : public QObject
{
  Q_OBJECT

  public:
    LdapSearch();

    static KConfig *config();
    static void readConfig( LdapServer &server, KConfig *config, int num, bool active );
    static void writeConfig( const LdapServer &server, KConfig *config, int j, bool active );

    void startSearch( const TQString& txt );
    void cancelSearch();
    bool isAvailable() const;
    void updateCompletionWeights();

    TQValueList< LdapClient* > clients() const { return mClients; }

  signals:
    /// Results, assembled as "Full Name <email>"
    /// (This signal can be emitted many times)
    void searchData( const TQStringList& );
    /// Another form for the results, with separate fields
    /// (This signal can be emitted many times)
    void searchData( const KPIM::LdapResultList& );
    void searchDone();

  private slots:
    void slotLDAPResult( const KPIM::LdapObject& );
    void slotLDAPError( const TQString& );
    void slotLDAPDone();
    void slotDataTimer();
    void slotFileChanged( const TQString& );

  private:
    void readWeighForClient( LdapClient *client, KConfig *config, int clientNumber );
    void readConfig();
    void finish();
    void makeSearchData( TQStringList& ret, LdapResultList& resList );
    TQValueList< LdapClient* > mClients;
    TQString mSearchText;
    TQTimer mDataTimer;
    int mActiveClients;
    bool mNoLDAPLookup;
    TQValueList< LdapObject > mResults;
    TQString mConfigFile;

  private:
    static KConfig *s_config;
    class LdapSearchPrivate* d;
};

}
#endif // KPIM_LDAPCLIENT_H
