/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ldapclient.h"
#include "ldapsession.h"
#include "ldapqueryjob.h"

#include <kldap/ldapobject.h>
#include <kldap/ldapserver.h>
#include <kldap/ldapurl.h>
#include <kldap/ldif.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KDirWatch>
#include <KProtocolInfo>
#include <KStandardDirs>
#include <kio/job.h>

#include <QtCore/QPointer>
#include <QtCore/QTimer>

using namespace KLDAP;

K_GLOBAL_STATIC_WITH_ARGS( KConfig, s_config, ( "kabldaprc", KConfig::NoGlobals ) )

class LdapClient::Private
{
  public:
    Private( LdapClient *qq )
      : q( qq ),
        mJob( 0 ),
        mActive( false ),
        mSession( 0 )
    {
    }

    ~Private()
    {
      q->cancelQuery();
#ifdef KDEPIM_INPROCESS_LDAP
      mSession->disconnectAndDelete();
      mSession = 0;
#endif
    }

    void startParseLDIF();
    void parseLDIF( const QByteArray &data );
    void endParseLDIF();
    void finishCurrentObject();

    void slotData( KIO::Job*, const QByteArray &data );
    void slotData( const QByteArray &data );
    void slotInfoMessage( KJob*, const QString &info, const QString& );
    void slotDone();

    LdapClient *q;

    KLDAP::LdapServer mServer;
    QString mScope;
    QStringList mAttrs;

    QPointer<KJob> mJob;
    bool mActive;

    KLDAP::LdapObject mCurrentObject;
    KLDAP::Ldif mLdif;
    int mClientNumber;
    int mCompletionWeight;

    KLDAP::LdapSession *mSession;
};

LdapClient::LdapClient( int clientNumber, QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->mClientNumber = clientNumber;
  d->mCompletionWeight = 50 - d->mClientNumber;
}

LdapClient::~LdapClient()
{
  delete d;
}

bool LdapClient::isActive() const
{
  return d->mActive;
}

void LdapClient::setServer( const KLDAP::LdapServer &server )
{
  d->mServer = server;
#ifdef KDEPIM_INPROCESS_LDAP
  if ( !d->mSession )
    d->mSession = new LdapSession( this );
  d->mSession->connectToServer( server );
#endif
}

const KLDAP::LdapServer LdapClient::server() const
{
  return d->mServer;
}

void LdapClient::setAttributes( const QStringList &attrs )
{
  d->mAttrs = attrs;
  d->mAttrs << "objectClass"; // via objectClass we detect distribution lists
}

QStringList LdapClient::attributes() const
{
  return d->mAttrs;
}

void LdapClient::setScope( const QString scope )
{
  d->mScope = scope;
}

void LdapClient::startQuery( const QString &filter )
{
  cancelQuery();
  KLDAP::LdapUrl url;

  url = d->mServer.url();

  url.setAttributes( d->mAttrs );
  url.setScope( d->mScope == "one" ? KLDAP::LdapUrl::One : KLDAP::LdapUrl::Sub );
  url.setFilter( '(' + filter + ')' );

  kDebug(5300) <<"LdapClient: Doing query:" << url.prettyUrl();

  d->startParseLDIF();
  d->mActive = true;
#ifndef KDEPIM_INPROCESS_LDAP
  d->mJob = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  connect( d->mJob, SIGNAL(data(KIO::Job*,QByteArray)),
           this, SLOT(slotData(KIO::Job*,QByteArray)) );
#else
  if ( !d->mSession )
    return;
  d->mJob = d->mSession->get( url );
  connect( d->mJob, SIGNAL(data(QByteArray)),
           this, SLOT(slotData(QByteArray)) );
#endif
  connect( d->mJob, SIGNAL(infoMessage(KJob*,QString,QString)),
           this, SLOT(slotInfoMessage(KJob*,QString,QString)) );
  connect( d->mJob, SIGNAL(result(KJob*)),
           this, SLOT(slotDone()) );
}

void LdapClient::cancelQuery()
{
  if ( d->mJob ) {
    d->mJob->kill();
    d->mJob = 0;
  }

  d->mActive = false;
}

void LdapClient::Private::slotData( KIO::Job*, const QByteArray &data )
{
  parseLDIF( data );
}

void LdapClient::Private::slotData( const QByteArray &data )
{
  parseLDIF( data );
}

void LdapClient::Private::slotInfoMessage( KJob*, const QString&, const QString& )
{
  //qDebug("Job said \"%s\"", info.toLatin1());
}

void LdapClient::Private::slotDone()
{
  endParseLDIF();
  mActive = false;
  if ( !mJob )
    return;
  int err = mJob->error();
  if ( err && err != KIO::ERR_USER_CANCELED ) {
    emit q->error( mJob->errorString() );
  }
#ifdef KDEPIM_INPROCESS_LDAP
  QMetaObject::invokeMethod( mJob, "deleteLater", Qt::QueuedConnection ); // it's in a different thread
#endif
  emit q->done();
}

void LdapClient::Private::startParseLDIF()
{
  mCurrentObject.clear();
  mLdif.startParsing();
}

void LdapClient::Private::endParseLDIF()
{
}

void LdapClient::Private::finishCurrentObject()
{
  mCurrentObject.setDn( mLdif.dn() );
  KLDAP::LdapAttrValue objectclasses;
  for ( KLDAP::LdapAttrMap::ConstIterator it = mCurrentObject.attributes().constBegin();
    it != mCurrentObject.attributes().constEnd(); ++it ) {

    if ( it.key().toLower() == "objectclass" ) {
      objectclasses = it.value();
      break;
    }
  }

  bool groupofnames = false;
  for ( KLDAP::LdapAttrValue::ConstIterator it = objectclasses.constBegin();
    it != objectclasses.constEnd(); ++it ) {

    QByteArray sClass = (*it).toLower();
    if ( sClass == "groupofnames" || sClass == "kolabgroupofnames" ) {
      groupofnames = true;
    }
  }

  if ( groupofnames ) {
    KLDAP::LdapAttrMap::ConstIterator it = mCurrentObject.attributes().find( "mail" );
    if ( it == mCurrentObject.attributes().end() ) {
      // No explicit mail address found so far?
      // Fine, then we use the address stored in the DN.
      QString sMail;
      QStringList lMail = mCurrentObject.dn().toString().split( ",dc=", QString::SkipEmptyParts );
      const int n = lMail.count();
      if ( n ) {
        if ( lMail.first().toLower().startsWith( QLatin1String( "cn=" ) ) ) {
          sMail = lMail.first().simplified().mid( 3 );
          if ( 1 < n ) {
            sMail.append( '@' );
          }
          for ( int i = 1; i < n; ++i ) {
            sMail.append( lMail[i] );
            if ( i < n - 1 ) {
              sMail.append( '.' );
            }
          }
          mCurrentObject.addValue( "mail", sMail.toUtf8() );
        }
      }
    }
  }
  emit q->result( *q, mCurrentObject );
  mCurrentObject.clear();
}

void LdapClient::Private::parseLDIF( const QByteArray &data )
{
  //kDebug(5300) <<"LdapClient::parseLDIF(" << QCString(data.data(), data.size()+1) <<" )";
  if ( data.size() ) {
    mLdif.setLdif( data );
  } else {
    mLdif.endLdif();
  }
  KLDAP::Ldif::ParseValue ret;
  QString name;
  do {
    ret = mLdif.nextItem();
    switch ( ret ) {
      case KLDAP::Ldif::Item:
        {
          name = mLdif.attr();
          QByteArray value = mLdif.value();
          mCurrentObject.addValue( name, value );
        }
        break;
     case KLDAP::Ldif::EndEntry:
        finishCurrentObject();
        break;
      default:
        break;
    }
  } while ( ret != KLDAP::Ldif::MoreData );
}

int LdapClient::clientNumber() const
{
  return d->mClientNumber;
}

int LdapClient::completionWeight() const
{
  return d->mCompletionWeight;
}

void LdapClient::setCompletionWeight( int weight )
{
  d->mCompletionWeight = weight;
}

void LdapClientSearch::readConfig( KLDAP::LdapServer &server, const KConfigGroup &config,
                                   int j, bool active )
{
  QString prefix;
  if ( active ) {
    prefix = "Selected";
  }

  const QString host =  config.readEntry( prefix + QString( "Host%1" ).arg( j ),
                                          QString() ).trimmed();
  if ( !host.isEmpty() ) {
    server.setHost( host );
  }

  const int port = config.readEntry( prefix + QString( "Port%1" ).arg( j ), 389 );
  server.setPort( port );

  const QString base = config.readEntry( prefix + QString( "Base%1" ).arg( j ),
                                         QString() ).trimmed();
  if ( !base.isEmpty() ) {
    server.setBaseDn( KLDAP::LdapDN( base ) );
  }

  const QString user = config.readEntry( prefix + QString( "User%1" ).arg( j ),
                                         QString() ).trimmed();
  if ( !user.isEmpty() ) {
    server.setUser( user );
  }

  QString bindDN = config.readEntry( prefix + QString( "Bind%1" ).arg( j ), QString() ).trimmed();
  if ( !bindDN.isEmpty() ) {
    server.setBindDn( bindDN );
  }

  QString pwdBindDN = config.readEntry( prefix + QString( "PwdBind%1" ).arg( j ), QString() );
  if ( !pwdBindDN.isEmpty() ) {
    server.setPassword( pwdBindDN );
  }

  server.setTimeLimit( config.readEntry( prefix + QString( "TimeLimit%1" ).arg( j ), 0 ) );
  server.setSizeLimit( config.readEntry( prefix + QString( "SizeLimit%1" ).arg( j ), 0 ) );
  server.setPageSize( config.readEntry( prefix + QString( "PageSize%1" ).arg( j ), 0 ) );
  server.setVersion( config.readEntry( prefix + QString( "Version%1" ).arg( j ), 3 ) );

  QString tmp;
  tmp = config.readEntry( prefix + QString( "Security%1" ).arg( j ),
                          QString::fromLatin1( "None" ) );
  server.setSecurity( KLDAP::LdapServer::None );
  if ( tmp == "SSL" ) {
    server.setSecurity( KLDAP::LdapServer::SSL );
  } else if ( tmp == "TLS" ) {
    server.setSecurity( KLDAP::LdapServer::TLS );
  }

  tmp = config.readEntry( prefix + QString( "Auth%1" ).arg( j ),
                          QString::fromLatin1( "Anonymous" ) );
  server.setAuth( KLDAP::LdapServer::Anonymous );
  if ( tmp == "Simple" ) {
    server.setAuth( KLDAP::LdapServer::Simple );
  } else if ( tmp == "SASL" ) {
    server.setAuth( KLDAP::LdapServer::SASL );
  }

  server.setMech( config.readEntry( prefix + QString( "Mech%1" ).arg( j ), QString() ) );
}

void LdapClientSearch::writeConfig( const KLDAP::LdapServer &server, KConfigGroup &config, int j,
                                    bool active )
{
  QString prefix;
  if ( active ) {
    prefix = "Selected";
  }

  config.writeEntry( prefix + QString( "Host%1" ).arg( j ), server.host() );
  config.writeEntry( prefix + QString( "Port%1" ).arg( j ), server.port() );
  config.writeEntry( prefix + QString( "Base%1" ).arg( j ), server.baseDn().toString() );
  config.writeEntry( prefix + QString( "User%1" ).arg( j ), server.user() );
  config.writeEntry( prefix + QString( "Bind%1" ).arg( j ), server.bindDn() );
  config.writeEntry( prefix + QString( "PwdBind%1" ).arg( j ), server.password() );
  config.writeEntry( prefix + QString( "TimeLimit%1" ).arg( j ), server.timeLimit() );
  config.writeEntry( prefix + QString( "SizeLimit%1" ).arg( j ), server.sizeLimit() );
  config.writeEntry( prefix + QString( "PageSize%1" ).arg( j ), server.pageSize() );
  config.writeEntry( prefix + QString( "Version%1" ).arg( j ), server.version() );
  QString tmp;
  switch ( server.security() ) {
    case KLDAP::LdapServer::TLS:
      tmp = "TLS";
      break;
    case KLDAP::LdapServer::SSL:
      tmp = "SSL";
      break;
    default:
      tmp = "None";
  }
  config.writeEntry( prefix + QString( "Security%1" ).arg( j ), tmp );
  switch ( server.auth() ) {
    case KLDAP::LdapServer::Simple:
      tmp = "Simple";
      break;
    case KLDAP::LdapServer::SSL:
      tmp = "SASL";
      break;
    default:
      tmp = "Anonymous";
  }
  config.writeEntry( prefix + QString( "Auth%1" ).arg( j ), tmp );
  config.writeEntry( prefix + QString( "Mech%1" ).arg( j ), server.mech() );
}

KConfig* LdapClientSearch::config()
{
  return s_config;
}

class LdapClientSearch::Private
{
  public:
    Private( LdapClientSearch *qq )
      : q( qq ), mActiveClients( 0 ), mNoLDAPLookup( false )
    {
    }

    struct ResultObject {
      const LdapClient *client;
      KLDAP::LdapObject object;
    };

    void readWeighForClient( LdapClient *client, const KConfigGroup &config, int clientNumber );
    void readConfig();
    void finish();
    void makeSearchData( QStringList &ret, LdapResult::List &resList );

    void slotLDAPResult( const KLDAP::LdapClient &client, const KLDAP::LdapObject& );
    void slotLDAPError( const QString& );
    void slotLDAPDone();
    void slotDataTimer();
    void slotFileChanged( const QString& );

    LdapClientSearch *q;
    QList<LdapClient*> mClients;
    QString mSearchText;
    QTimer mDataTimer;
    int mActiveClients;
    bool mNoLDAPLookup;
    QList<ResultObject> mResults;
    QString mConfigFile;
};

LdapClientSearch::LdapClientSearch( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
#ifndef Q_OS_WINCE
// There is no KSycoca on WinCE so this would always fail
  if ( !KProtocolInfo::isKnownProtocol( KUrl( "ldap://localhost" ) ) ) {
    d->mNoLDAPLookup = true;
    return;
  }
#endif

  d->readConfig();
  connect( KDirWatch::self(), SIGNAL(dirty(QString)), this,
           SLOT(slotFileChanged(QString)) );
}

LdapClientSearch::~LdapClientSearch()
{
  delete d;
}

void LdapClientSearch::Private::readWeighForClient( LdapClient *client, const KConfigGroup &config,
                                                    int clientNumber )
{
  const int completionWeight = config.readEntry( QString( "SelectedCompletionWeight%1" ).arg( clientNumber ), -1 );
  if ( completionWeight != -1 ) {
    client->setCompletionWeight( completionWeight );
  }
}

void LdapClientSearch::updateCompletionWeights()
{
  KConfigGroup config( KLDAP::LdapClientSearch::config(), "LDAP" );
  for ( int i = 0; i < d->mClients.size(); i++ ) {
    d->readWeighForClient( d->mClients[ i ], config, i );
  }
}

QList<LdapClient*> LdapClientSearch::clients() const
{
  return d->mClients;
}

void LdapClientSearch::Private::readConfig()
{
  q->cancelSearch();
  qDeleteAll( mClients );
  mClients.clear();

  // stolen from KAddressBook
  KConfigGroup config( KLDAP::LdapClientSearch::config(), "LDAP" );
  const int numHosts = config.readEntry( "NumSelectedHosts", 0 );
  if ( !numHosts ) {
    mNoLDAPLookup = true;
  } else {
    for ( int j = 0; j < numHosts; j++ ) {
      LdapClient *ldapClient = new LdapClient( j, q );
      KLDAP::LdapServer server;
      q->readConfig( server, config, j, true );
      if ( !server.host().isEmpty() ) {
        mNoLDAPLookup = false;
      }
      ldapClient->setServer( server );

      readWeighForClient( ldapClient, config, j );

      QStringList attrs;
      attrs << "cn" << "mail" << "givenname" << "sn";
      ldapClient->setAttributes( attrs );

      q->connect( ldapClient, SIGNAL(result(KLDAP::LdapClient,KLDAP::LdapObject)),
                  q, SLOT(slotLDAPResult(KLDAP::LdapClient,KLDAP::LdapObject)) );
      q->connect( ldapClient, SIGNAL(done()),
                  q, SLOT(slotLDAPDone()) );
      q->connect( ldapClient, SIGNAL(error(QString)),
                  q, SLOT(slotLDAPError(QString)) );

      mClients.append( ldapClient );
    }

    q->connect( &mDataTimer, SIGNAL(timeout()), SLOT(slotDataTimer()) );
  }
  mConfigFile = KStandardDirs::locateLocal( "config", "kabldaprc" );
  KDirWatch::self()->addFile( mConfigFile );
}

void LdapClientSearch::Private::slotFileChanged( const QString &file )
{
  if ( file == mConfigFile ) {
    readConfig();
  }
}

void LdapClientSearch::startSearch( const QString &txt )
{
  if ( d->mNoLDAPLookup ) {
    return;
  }

  cancelSearch();

  int pos = txt.indexOf( '\"' );
  if ( pos >= 0 ) {
    ++pos;
    const int pos2 = txt.indexOf( '\"', pos );
    if ( pos2 >= 0 ) {
        d->mSearchText = txt.mid( pos, pos2 - pos );
    } else {
        d->mSearchText = txt.mid( pos );
    }
  } else {
    d->mSearchText = txt;
  }

  /* The reasoning behind this filter is:
   * If it's a person, or a distlist, show it, even if it doesn't have an email address.
   * If it's not a person, or a distlist, only show it if it has an email attribute.
   * This allows both resource accounts with an email address which are not a person and
   * person entries without an email address to show up, while still not showing things
   * like structural entries in the ldap tree. */
  const QString filter = QString( "&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                  "(|(cn=%1*)(mail=%2*)(mail=*@%3*)(givenName=%4*)(sn=%5*))" )
                                .arg( d->mSearchText ).arg( d->mSearchText )
                                .arg( d->mSearchText ).arg( d->mSearchText ).arg( d->mSearchText );

  QList<LdapClient*>::Iterator it;
  for ( it = d->mClients.begin(); it != d->mClients.end(); ++it ) {
    (*it)->startQuery( filter );
    kDebug(5300) <<"LdapClientSearch::startSearch()" << filter;
    d->mActiveClients++;
  }
}

void LdapClientSearch::cancelSearch()
{
  QList<LdapClient*>::Iterator it;
  for ( it = d->mClients.begin(); it != d->mClients.end(); ++it ) {
    (*it)->cancelQuery();
  }

  d->mActiveClients = 0;
  d->mResults.clear();
}

void LdapClientSearch::Private::slotLDAPResult( const LdapClient &client,
                                                const KLDAP::LdapObject &obj )
{
  ResultObject result;
  result.client = &client;
  result.object = obj;

  mResults.append( result );
  if ( !mDataTimer.isActive() ) {
    mDataTimer.setSingleShot( true );
    mDataTimer.start( 500 );
  }
}

void LdapClientSearch::Private::slotLDAPError( const QString& )
{
  slotLDAPDone();
}

void LdapClientSearch::Private::slotLDAPDone()
{
  if ( --mActiveClients > 0 ) {
    return;
  }

  finish();
}

void LdapClientSearch::Private::slotDataTimer()
{
  QStringList lst;
  LdapResult::List reslist;
  makeSearchData( lst, reslist );
  if ( !lst.isEmpty() ) {
    emit q->searchData( lst );
  }
  if ( !reslist.isEmpty() ) {
    emit q->searchData( reslist );
  }
}

void LdapClientSearch::Private::finish()
{
  mDataTimer.stop();

  slotDataTimer(); // emit final bunch of data
  emit q->searchDone();
}

void LdapClientSearch::Private::makeSearchData( QStringList &ret, LdapResult::List &resList )
{
  QString search_text_upper = mSearchText.toUpper();

  QList< ResultObject >::ConstIterator it1;
  for ( it1 = mResults.constBegin(); it1 != mResults.constEnd(); ++it1 ) {
    QString name, mail, givenname, sn;
    QStringList mails;
    bool isDistributionList = false;
    bool wasCN = false;
    bool wasDC = false;

    //kDebug(5300) <<"\n\nLdapClientSearch::makeSearchData()";

    KLDAP::LdapAttrMap::ConstIterator it2;
    for ( it2 = (*it1).object.attributes().constBegin();
          it2 != (*it1).object.attributes().constEnd(); ++it2 ) {
      QByteArray val = (*it2).first();
      int len = val.size();
      if ( len > 0 && '\0' == val[len-1] ) {
        --len;
      }
      const QString tmp = QString::fromUtf8( val, len );
      //kDebug(5300) <<"      key: \"" << it2.key() <<"\" value: \"" << tmp <<"\"";
      if ( it2.key() == "cn" ) {
        name = tmp;
        if ( mail.isEmpty() ) {
          mail = tmp;
        } else {
          if ( wasCN ) {
            mail.prepend( "." );
          } else {
            mail.prepend( "@" );
          }
          mail.prepend( tmp );
        }
        wasCN = true;
      } else if ( it2.key() == "dc" ) {
        if ( mail.isEmpty() ) {
          mail = tmp;
        } else {
          if ( wasDC ) {
            mail.append( "." );
          } else {
            mail.append( "@" );
          }
          mail.append( tmp );
        }
        wasDC = true;
      } else if ( it2.key() == "mail" ) {
        mail = tmp;
        KLDAP::LdapAttrValue::ConstIterator it3 = it2.value().constBegin();
        for ( ; it3 != it2.value().constEnd(); ++it3 ) {
          mails.append( QString::fromUtf8( (*it3).data(), (*it3).size() ) );
        }
      } else if ( it2.key() == "givenName" ) {
        givenname = tmp;
      } else if ( it2.key() == "sn" ) {
        sn = tmp;
      } else if ( it2.key() == "objectClass" &&
               (tmp == "groupOfNames" || tmp == "kolabGroupOfNames") ) {
        isDistributionList = true;
      }
    }

    if ( mails.isEmpty() ) {
      if ( !mail.isEmpty() ) {
        mails.append( mail );
      }
      if ( isDistributionList ) {
        //kDebug(5300) <<"\n\nLdapClientSearch::makeSearchData() found a list:" << name;
        ret.append( name );
        // following lines commented out for bugfixing kolab issue #177:
        //
        // Unlike we thought previously we may NOT append the server name here.
        //
        // The right server is found by the SMTP server instead: Kolab users
        // must use the correct SMTP server, by definition.
        //
        //mail = (*it1).client->base().simplified();
        //mail.replace( ",dc=", ".", false );
        //if( mail.startsWith("dc=", false) )
        //  mail.remove(0, 3);
        //mail.prepend( '@' );
        //mail.prepend( name );
        //mail = name;
      } else {
        continue; // nothing, bad entry
      }
    } else if ( name.isEmpty() ) {
      ret.append( mail );
    } else {
      ret.append( QString( "%1 <%2>" ).arg( name ).arg( mail ) );
    }

    LdapResult sr;
    sr.clientNumber = (*it1).client->clientNumber();
    sr.completionWeight = (*it1).client->completionWeight();
    sr.name = name;
    sr.email = mails;
    resList.append( sr );
  }

  mResults.clear();
}

bool LdapClientSearch::isAvailable() const
{
  return !d->mNoLDAPLookup;
}

#include "ldapclient.moc"
