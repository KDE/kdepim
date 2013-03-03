/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
 *
 * Copyright (C) 2013 Laurent Montel <montel@kde.org>
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

#include "ldapclientsearch.h"
#include "ldapclientsearchconfig.h"

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

#if 0
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
#endif
KConfig* LdapClientSearch::config()
{
  return s_config;
}

class LdapClientSearch::Private
{
  public:
    Private( LdapClientSearch *qq )
        : q( qq ),
          mActiveClients( 0 ),
          mNoLDAPLookup( false )
    {
        mClientSearchConfig = new LdapClientSearchConfig;
    }

    ~Private()
    {
        delete mClientSearchConfig;
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
    LdapClientSearchConfig *mClientSearchConfig;
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
      mClientSearchConfig->readConfig( server, config, j, true );
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

#include "ldapclientsearch.moc"
