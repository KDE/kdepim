/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

// The namespace mapping table is required and associates namespace prefixes
// with namespace names:
#include "GroupWiseBinding.nsmap"

#include <libkcal/calendar.h>
#include <libkcal/incidence.h>
#include <libkdepim/kpimprefs.h>

#include <kabc/addressee.h>
#include <kabc/addresseelist.h>

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kprotocolmanager.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <klocale.h>

#include <qnamespace.h>
#include <qfile.h>

#include "ksslsocket.h"
#include "contactconverter.h"
#include "incidenceconverter.h"
#include "kcal_resourcegroupwise.h"
#include "soapH.h"
#include "stdsoap2.h"
#include "soapGroupWiseBindingProxy.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "groupwiseserver.h"

static QMap<struct soap *,GroupwiseServer *> mServerMap;

int myOpen( struct soap *soap, const char *endpoint, const char *host, int port )
{
  QMap<struct soap *,GroupwiseServer *>::ConstIterator it;
  it = mServerMap.find( soap );
  if ( it == mServerMap.end() ) {
    soap->error = SOAP_FAULT;
    return SOAP_INVALID_SOCKET;
  }

  return (*it)->gSoapOpen( soap, endpoint, host, port );
}

int myClose( struct soap *soap )
{
  QMap<struct soap *,GroupwiseServer *>::ConstIterator it;
  it = mServerMap.find( soap );
  if ( it == mServerMap.end() ) return SOAP_FAULT;

  return (*it)->gSoapClose( soap );
}

int mySendCallback( struct soap *soap, const char *s, size_t n )
{
  QMap<struct soap *,GroupwiseServer *>::ConstIterator it;
  it = mServerMap.find( soap );
  if ( it == mServerMap.end() ) return SOAP_FAULT;

  return (*it)->gSoapSendCallback( soap, s, n );
}

size_t myReceiveCallback( struct soap *soap, char *s, size_t n )
{
  QMap<struct soap *,GroupwiseServer *>::ConstIterator it;
  it = mServerMap.find( soap );
  if ( it == mServerMap.end() ) {
    kdDebug() << "No soap object found" << endl;
    soap->error = SOAP_FAULT;
    return 0;
  }

  return (*it)->gSoapReceiveCallback( soap, s, n );
}

int GroupwiseServer::gSoapOpen( struct soap *soap, const char *,
  const char *host, int port )
{
//  kdDebug() << "GroupwiseServer::gSoapOpen()" << endl;

  if ( m_sock ) {
    kdError() << "m_sock non-null: " << (void*)m_sock << endl;
    delete m_sock;
  }

  if ( mSSL ) {
//    kdDebug() << "Creating KSSLSocket()" << endl;
    m_sock = new KSSLSocket();
    m_sock->setTimeout( KProtocolManager::connectTimeout() );
    connect( m_sock, SIGNAL( sslFailure() ), SLOT( slotSslError() ) );
  } else {
    m_sock = new KExtendedSocket();
  }
  mErrorText = QString::null;

  m_sock->reset();
  m_sock->setBlockingMode( false );
  m_sock->setSocketFlags( KExtendedSocket::inetSocket );

  m_sock->setAddress( host, port );
  int rc = m_sock->lookup();
  if (rc != 0 ) {
    kdError() << "gSoapOpen: lookup of " << host << " failed " << rc << endl;
    QString errorMessage;
    errorMessage = QString::fromLatin1( strerror( errno ) );
    perror( 0 );
    soap->error = SOAP_TCP_ERROR;
    mErrorText = i18n("Connect failed: %1.").arg( errorMessage );
    return SOAP_INVALID_SOCKET;
  }

  rc = m_sock->connect();
  if ( rc != 0 ) {
    kdError() << "gSoapOpen: connect failed " << rc << endl;
    QString errorMessage;
    if ( rc == -1 ) {
      errorMessage = QString::fromLatin1( strerror( errno ) );
      perror( 0 );
    }
	//set the soap struct's error here!
    else {
      if ( rc == -3 )
        errorMessage = QString::fromLatin1( "Connection timed out.  Check host and port number" );
    }
    mErrorText = i18n("Connect failed: %1.").arg( errorMessage );
    soap->error =SOAP_TCP_ERROR;
   return SOAP_INVALID_SOCKET;
  }
  m_sock->enableRead( true );
  m_sock->enableWrite( true );

  // hopefully never really used by SOAP
#if 0
  return m_sock->fd();
#else
  return 0;
#endif
}

int GroupwiseServer::gSoapClose( struct soap * )
{
//  kdDebug() << "GroupwiseServer::gSoapClose()" << endl;

  delete m_sock;
  m_sock = 0;

#if 0
   m_sock->close();
   m_sock->reset();
#endif
   return SOAP_OK;
}

int GroupwiseServer::gSoapSendCallback( struct soap * soap, const char *s, size_t n )
{
//  kdDebug() << "GroupwiseServer::gSoapSendCallback()" << endl;

  if ( !m_sock ) {
    kdError() << "no open connection" << endl;
    soap->error = SOAP_TCP_ERROR;
    return SOAP_TCP_ERROR;
  }
  if ( mError ) {
    kdError() << "SSL is in error state." << endl;
     soap->error = SOAP_SSL_ERROR;
     return SOAP_SSL_ERROR;
  }

  if ( getenv("DEBUG_GW_RESOURCE") ) {
    qDebug("*************************");
    char p[99999];
    strncpy(p, s, n);
    p[n]='\0';
    qDebug("%s", p );
    qDebug("\n*************************");
  }
  log( "SENT", s, n );

  int ret;
  while ( n > 0 ) {
    ret = m_sock->writeBlock( s, n );
    if ( ret < 0 ) {
      kdError() << "Send failed: " << strerror( m_sock->systemError() )
        << " " << m_sock->socketStatus() << " " << m_sock->fd() << endl;
      soap->error = SOAP_TCP_ERROR;
      return SOAP_TCP_ERROR;
    }
    n -= ret;
  }

  if ( n !=0 ) {
    kdError() << "Send failed: " << strerror( m_sock->systemError() )
      << " " << m_sock->socketStatus() << " " << m_sock->fd() << endl;
    soap->error = SOAP_TCP_ERROR;
  }

  m_sock->flush();

  return SOAP_OK;
}

size_t GroupwiseServer::gSoapReceiveCallback( struct soap *soap, char *s,
  size_t n )
{
//  kdDebug() << "GroupwiseServer::gSoapReceiveCallback()" << endl;

  if ( !m_sock ) {
    kdError() << "no open connection" << endl;
    soap->error = SOAP_FAULT;
    return 0;
  }
  if ( mError ) {
    kdError() << "SSL is in error state." << endl;
    soap->error = SOAP_SSL_ERROR;
    return 0;
  }

//   m_sock->open();
  long ret = m_sock->readBlock( s, n );
  if ( ret < 0 ) {
    kdError() << "Receive failed: " << strerror( m_sock->systemError() )
      << " " << m_sock->socketStatus() << " " << m_sock->fd() << endl;
  } else {
    if ( getenv("DEBUG_GW_RESOURCE") ) {
      qDebug("*************************");
      char p[99999];
      strncpy(p, s, ret);
      p[ret]='\0';
      qDebug("%s", p );
      qDebug("\n*************************");
      qDebug("kioReceiveCallback return %ld", ret);
    }
    log( "RECV", s, ret );
  }

  return ret;
}

GroupwiseServer::GroupwiseServer( const QString &url, const QString &user,
                                  const QString &password, QObject *parent )
  : QObject( parent, "GroupwiseServer" ),
    mUrl( url ), mUser( user ), mPassword( password ),
    mSSL( url.left(6)=="https:" ), m_sock( 0 ),
    mError( 0 )
{
  mBinding = new GroupWiseBinding;
  mSoap = mBinding->soap;

  kdDebug() << "GroupwiseServer(): URL: " << url << endl;

  soap_init( mSoap );

#if 1
  // disable this block to use native gSOAP network functions
  mSoap->fopen = myOpen;
  mSoap->fsend = mySendCallback;
  mSoap->frecv = myReceiveCallback;
  mSoap->fclose = myClose;

  KConfig cfg( "groupwiserc" );
  cfg.setGroup( "Debug" );
  mLogFile = cfg.readEntry( "LogFile" );
  
  if ( !mLogFile.isEmpty() ) {
    kdDebug() << "Debug log file enabled: " << mLogFile << endl;
  }
#endif

  mServerMap.insert( mSoap, this );
}

GroupwiseServer::~GroupwiseServer()
{
  delete mSoap;
  mSoap = 0;
}

bool GroupwiseServer::login()
{
  _ngwm__loginResponse loginResp;
  _ngwm__loginRequest loginReq;
  loginReq.application = soap_new_std__string( mSoap, -1 );
  loginReq.application->append( "KDEPIM" );
  loginReq.language.append( "us" );
  loginReq.version.append( "1" );
  
  GWConverter conv( mSoap );

  ngwt__PlainText pt;

  pt.username = mUser.utf8();
  pt.password = conv.qStringToString( mPassword );
  loginReq.auth = &pt;
  mSoap->userid = strdup( mUser.utf8() );
  mSoap->passwd = strdup( mPassword.utf8() );

  mSession = "";

  kdDebug() << "GroupwiseServer::login() URL: " << mUrl << endl;

  int result = 1, maxTries = 3;
  mBinding->endpoint = mUrl.latin1();
  
//  while ( --maxTries && result ) {
    result = soap_call___ngw__loginRequest( mSoap, mUrl.latin1(), NULL,
      &loginReq, &loginResp );
    /*result = mBinding->__ngw__loginRequest( &loginReq, &loginResp );*/
//  }

  if ( !checkResponse( result, loginResp.status ) ) return false;

  mSession = *(loginResp.session);

  if ( mSession.size() == 0 ) // workaround broken loginResponse error reporting
  {
    kdDebug() << "Login failed but the server didn't report an error" << endl;
    mErrorText = i18n( "Login failed, but the GroupWise server did not report an error" );
    return false;
  }

  mSoap->header = new( SOAP_ENV__Header );

  mUserName = "";
  mUserEmail = "";
  mUserUuid = "";

  ngwt__UserInfo *userinfo = loginResp.userinfo;
  if ( userinfo ) {
    kdDebug() << "HAS USERINFO" << endl;
    mUserName = conv.stringToQString( userinfo->name );
    if ( userinfo->email ) mUserEmail = conv.stringToQString( userinfo->email );
    if ( userinfo->uuid ) mUserUuid = conv.stringToQString( userinfo->uuid );
    // can also get userid here in GW7 (userinfo->userid)
  }

  kdDebug() << "USER: name: " << mUserName << " email: " << mUserEmail <<
    " uuid: " << mUserUuid << endl;

  return true;
}

bool GroupwiseServer::getCategoryList()
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::getCategoryList(): no session." << endl;
    return false;
  }
  
/*SOAP_FMAC5 int SOAP_FMAC6 soap_call___ngw__getCategoryListRequest(struct soap *soap, const char *soap_endpoint, const char *soap_action, _ngwm__getCategoryListRequest *ngwm__getCategoryListRequest, _ngwm__getCategoryListResponse *ngwm__getCategoryListResponse);*/

  _ngwm__getCategoryListRequest catListReq;
  _ngwm__getCategoryListResponse catListResp;
  mSoap->header->ngwt__session = mSession;
  int result = soap_call___ngw__getCategoryListRequest( mSoap, mUrl.latin1(),
    0, &catListReq, &catListResp);
  if ( !checkResponse( result, catListResp.status ) ) return false;

  if ( catListResp.categories ) {
    std::vector<class ngwt__Category * > *categories;
    categories = &catListResp.categories->category;
    std::vector<class ngwt__Category * >::const_iterator it;
    for( it = categories->begin(); it != categories->end(); ++it ) {
//      kdDebug() << "CATEGORY" << endl;
      dumpItem( *it );
    }
  }

  return true;
}

bool GroupwiseServer::dumpData()
{
  mSoap->header->ngwt__session = mSession;
  _ngwm__getAddressBookListRequest addressBookListRequest;
  _ngwm__getAddressBookListResponse addressBookListResponse;
  soap_call___ngw__getAddressBookListRequest( mSoap, mUrl.latin1(),
                                              NULL, &addressBookListRequest, &addressBookListResponse );
  soap_print_fault(mSoap, stderr);

  if ( addressBookListResponse.books ) {
    std::vector<class ngwt__AddressBook * > *addressBooks = &addressBookListResponse.books->book;
    std::vector<class ngwt__AddressBook * >::const_iterator it;
    for( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      ngwt__AddressBook *book = *it;
      if ( book->description ) {
        kdDebug() << "ADDRESSBOOK: description: " << book->description->c_str() << endl;
      }
      if ( book->id ) {
        kdDebug() << "ADDRESSBOOK: id: " << book->id->c_str() << endl;
      }
      if ( book->name ) {
        kdDebug() << "ADDRESSBOOK: name: " << book->name->c_str() << endl;
      }

      _ngwm__getItemsRequest itemsRequest;
      if ( !book->id ) {
        kdError() << "Missing book id" << endl;
      } else {
        itemsRequest.container = book->id;
      }
      itemsRequest.filter = 0;
      itemsRequest.items = 0;
//      itemsRequest.count = -1;

      mSoap->header->ngwt__session = mSession;
      _ngwm__getItemsResponse itemsResponse;
      soap_call___ngw__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                        &itemsRequest,
                                        &itemsResponse );

      std::vector<class ngwt__Item * > *items = &itemsResponse.items->item;
      if ( items ) {
        std::vector<class ngwt__Item * >::const_iterator it2;
        for( it2 = items->begin(); it2 != items->end(); ++it2 ) {
          kdDebug() << "ITEM" << endl;
          dumpItem( *it2 );

          if ( true ) {
            _ngwm__getItemRequest itemRequest;
            if ( !(*it2)->id ) {
              kdError() << "Missing item id" << endl;
            } else {
              itemRequest.id = *( (*it2)->id );
            }
            itemRequest.view = 0;

            mSoap->header->ngwt__session = mSession;
            _ngwm__getItemResponse itemResponse;
            soap_call___ngw__getItemRequest( mSoap, mUrl.latin1(), 0,
                                             &itemRequest,
                                             &itemResponse );

            ngwt__Item *item = itemResponse.item;
            ngwt__Contact *contact = dynamic_cast<ngwt__Contact *>( item );
            if ( !contact ) {
              kdError() << "Cast failed." << endl;
            } else {
              kdDebug() << "Cast succeeded." << endl;
            }
          }
        }
      }
    }
  }

  return true;
}

void GroupwiseServer::dumpFolderList()
{
  mSoap->header->ngwt__session = mSession;
  _ngwm__getFolderListRequest folderListReq;
  folderListReq.parent = "folders";
  folderListReq.recurse = true;
  _ngwm__getFolderListResponse folderListRes;
  soap_call___ngw__getFolderListRequest( mSoap, mUrl.latin1(), 0,
                                         &folderListReq,
                                         &folderListRes );

  if ( folderListRes.folders ) {
    std::vector<class ngwt__Folder * > *folders = &folderListRes.folders->folder;
    if ( folders ) {
      std::vector<class ngwt__Folder * >::const_iterator it;
      for ( it = folders->begin(); it != folders->end(); ++it ) {
        kdDebug() << "FOLDER" << endl;
        dumpFolder( *it );
#if 0
        if ( (*it)->type && *((*it)->type) == "Calendar" ) {
          if ( !(*it)->id ) {
            kdError() << "Missing calendar id" << endl;
          } else {
            dumpCalendarFolder( *( (*it)->id ) );
          }
        }
#else
        if ( !(*it)->id ) {
          kdError() << "Missing calendar id" << endl;
        } else {
          dumpCalendarFolder( *( (*it)->id ) );
        }   
#endif
      }
    }
  }
}

void GroupwiseServer::dumpCalendarFolder( const std::string &id )
{
  _ngwm__getItemsRequest itemsRequest;

  itemsRequest.container = soap_new_std__string( mSoap, -1 );
  *(itemsRequest.container) = id;
  std::string *str = soap_new_std__string( mSoap, -1 );
  str->append( "recipients message recipientStatus" );
  itemsRequest.view = str;

  itemsRequest.filter = 0;
  itemsRequest.items = 0;
//  itemsRequest.count = 5;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getItemsResponse itemsResponse;
  soap_call___ngw__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  soap_print_fault(mSoap, stderr);

  std::vector<class ngwt__Item * > *items = &itemsResponse.items->item;

  if ( items ) {
    std::vector<class ngwt__Item * >::const_iterator it;
    for( it = items->begin(); it != items->end(); ++it ) {
#if 0
      if ( (*it)->type ) {
        kdDebug() << "ITEM type '" << (*it)->type->c_str() << "'" << endl;
      } else {
        kdDebug() << "ITEM no type" << endl;
      }
#endif
      ngwt__Appointment *a = dynamic_cast<ngwt__Appointment *>( *it );
      if ( !a ) {
        kdError() << "Appointment cast failed." << endl;
      } else {
        kdDebug() << "CALENDAR ITEM" << endl;
        dumpAppointment( a );
      }
      ngwt__Task *t = dynamic_cast<ngwt__Task *>( *it );
      if ( !t ) {
        kdError() << "Task cast failed." << endl;
      } else {
        kdDebug() << "TASK" << endl;
        dumpTask( t );
      }
    }
  }
}

void GroupwiseServer::dumpMail( ngwt__Mail *m )
{
  dumpItem( m );
  kdDebug() << "  SUBJECT: " << m->subject << endl;
}

void GroupwiseServer::dumpTask( ngwt__Task *t )
{
  dumpMail( t );
  if ( t->completed ) {
    kdDebug() << "  COMPLETED: " << ( t->completed ? "true" : "false" ) << endl; 
  }
}

void GroupwiseServer::dumpAppointment( ngwt__Appointment *a )
{
  dumpMail( a );
  kdDebug() << "  START DATE: " << a->startDate << endl;
  kdDebug() << "  END DATE: " << a->endDate << endl;
  if ( a->allDayEvent ) {
    kdDebug() << "  ALL DAY: " << ( a->allDayEvent ? "true" : "false" ) << endl;
  }
}

void GroupwiseServer::dumpFolder( ngwt__Folder *f )
{
  dumpItem( f );
  kdDebug() << "  PARENT: " << f->parent.c_str() << endl;
  if ( f->description ) {
    kdDebug() << "  DESCRIPTION: " << f->description->c_str() << endl;
  }
  // FIXME: More fields
//	int *count;
//	bool *hasUnread;
//	int *unreadCount;
//	unsigned long sequence;
//	std::string *settings;
//	bool *hasSubfolders;
//	ngwt__SharedFolderNotification *notification;
}

void GroupwiseServer::dumpItem( ngwt__Item *i )
{
  if ( !i ) return;
  if ( i->id ) {
    kdDebug() << "  ID: " << i->id->c_str() << endl;
  }
  if ( i->name ) {
    kdDebug() << "  NAME: " << i->name->c_str() << endl;
  }
  kdDebug() << "  VERSION: " << i->version << endl;
  kdDebug() << "  MODIFIED: " << i->modified << endl;
  if ( i->changes ) kdDebug() << "  HASCHANGES" << endl;
#if 0
  if ( i->type ) {
    kdDebug() << "  TYPE: " << i->type->c_str() << endl;
  }
#endif
}

bool GroupwiseServer::logout()
{
  // FIXME: Send logoutRequest
  mSoap->header->ngwt__session = mSession;
  _ngwm__logoutRequest request;
  _ngwm__logoutResponse response;

  int result = soap_call___ngw__logoutRequest( mSoap, mUrl.latin1(),
                                               NULL, &request, &response);
  soap_print_fault( mSoap, stderr );
  if (!checkResponse( result, response.status ) )
    kdDebug() << "error while logging out" << endl;

  soap_end( mSoap );
  soap_done( mSoap );

  delete mSoap->header;
  mSoap->header = 0;

  return true;
}

GroupWise::DeltaInfo GroupwiseServer::getDeltaInfo( const QStringList & addressBookIds )
{
  GroupWise::DeltaInfo info;
  info.count = 0;
  info.firstSequence = 0;
  info.lastSequence = 0;
  info.lastTimePORebuild = 0;

  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::getDeltaInfo(): no session." << endl;
    return info;
  }

  mSoap->header->ngwt__session = mSession;
  _ngwm__getDeltaInfoRequest request;
  _ngwm__getDeltaInfoResponse response;

  GWConverter conv( mSoap );
  request.container.append( addressBookIds.first().latin1() );

  int result = soap_call___ngw__getDeltaInfoRequest( mSoap, mUrl.latin1(),
                                              NULL, &request, &response);
  soap_print_fault( mSoap, stderr );
  if (!checkResponse( result, response.status ) )
    return info;

  if ( response.deltaInfo->count )
    info.count = *( response.deltaInfo->count );
  if ( response.deltaInfo->firstSequence )
    info.firstSequence = *( response.deltaInfo->firstSequence );
  if ( response.deltaInfo->lastSequence )
    info.lastSequence = *( response.deltaInfo->lastSequence );
  if ( response.deltaInfo->lastTimePORebuild )
    info.lastTimePORebuild = response.deltaInfo->lastTimePORebuild;

  return info;
}

GroupWise::AddressBook::List GroupwiseServer::addressBookList()
{
  GroupWise::AddressBook::List books;

  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::addressBookList(): no session." << endl;
    return books;
  }

  mSoap->header->ngwt__session = mSession;
  _ngwm__getAddressBookListRequest addressBookListRequest;
  _ngwm__getAddressBookListResponse addressBookListResponse;
  int result = soap_call___ngw__getAddressBookListRequest( mSoap, mUrl.latin1(),
    NULL, &addressBookListRequest, &addressBookListResponse );
  if ( !checkResponse( result, addressBookListResponse.status ) ) {
    return books;
  }

  if ( addressBookListResponse.books ) {
    std::vector<class ngwt__AddressBook * > *addressBooks = &addressBookListResponse.books->book;
    std::vector<class ngwt__AddressBook * >::const_iterator it;
    for ( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      GroupWise::AddressBook ab;
      ab.id = GWConverter::stringToQString( (*it)->id );
      ab.name = GWConverter::stringToQString( (*it)->name );
      ab.description = GWConverter::stringToQString( (*it)->description );
      if ( (*it)->isPersonal ) ab.isPersonal = (*it)->isPersonal;
      if ( (*it)->isFrequentContacts ) {
        ab.isFrequentContacts = (*it)->isFrequentContacts;
      }
      books.append( ab );
    }
  }

  return books;
}

bool GroupwiseServer::readAddressBooksSynchronous( const QStringList &addrBookIds )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::readAddressBooksSynchronous(): no session." << endl;
    return false;
  }

  ReadAddressBooksJob *job = new ReadAddressBooksJob( this, mSoap,
    mUrl, mSession );
  job->setAddressBookIds( addrBookIds );

  job->run();

  return true;
}

bool GroupwiseServer::updateAddressBooks( const QStringList &addrBookIds, const unsigned long startSequenceNumber, const unsigned long lastPORebuildTime )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::updateAddressBooks(): no session." << endl;
    return false;
  }

  UpdateAddressBooksJob * job = new UpdateAddressBooksJob( this, mSoap, mUrl, mSession );
  job->setAddressBookIds( addrBookIds );
  job->setStartSequenceNumber( startSequenceNumber );
  job->setLastPORebuildTime( lastPORebuildTime );

  job->run();
  if ( job->error() == GroupWise::RefreshNeeded )
  {
    mError = 1;
    mErrorText = "The System Address Book must be refreshed";
    return false;
  }
  return true;
}

std::string GroupwiseServer::getFullIDFor( const QString & gwRecordIDFromIcal )
{
  // first get the ID of the calendar folder - because we don't store this in the resource we have to fetch it manually
  std::string calendarFolderID;
  _ngwm__getFolderListRequest folderListReq;
  _ngwm__getFolderListResponse folderListRes;
  folderListReq.parent = "folders";
  folderListReq.view = soap_new_std__string( mSoap, -1 );
  folderListReq.view->append( "id type" );
  folderListReq.recurse = false;

  mSoap->header->ngwt__session = mSession;
  soap_call___ngw__getFolderListRequest( mSoap, mUrl.latin1(), 0,
                                         &folderListReq,
                                         &folderListRes );

  if ( folderListRes.folders ) {
    std::vector<class ngwt__Folder * > *folders = &folderListRes.folders->folder;
    if ( folders ) {
      std::vector<class ngwt__Folder * >::const_iterator it;
      for ( it = folders->begin(); it != folders->end(); ++it ) {
        ngwt__SystemFolder * fld = dynamic_cast<ngwt__SystemFolder *>( *it );
        if ( fld && *(fld->folderType) == Calendar )
          if ( !fld->id ) {
            kdError() << "No folder id" << endl;
          } else {
            calendarFolderID = *fld->id;
          }
      }
    }
  }
  if ( calendarFolderID.empty() )
  {
    kdError() << "couldn't get calendar folder ID in order to accept invitation" << endl;
    return std::string();
  }
  
  // now get the full Item ID of the 
  std::string fullItemID;

  _ngwm__getItemsRequest getItemRequest;
  _ngwm__getItemsResponse getItemResponse;
  //getItemRequest.id.append( gwRecordID.latin1() );
  getItemRequest.view = 0;
  getItemRequest.filter = soap_new_ngwt__Filter( mSoap, -1 );
  ngwt__FilterEntry * fe = soap_new_ngwt__FilterEntry( mSoap, -1 );
  fe->op = eq;
  fe->field = soap_new_std__string( mSoap, -1 );
  fe->field->append( "id" );
  fe->value = soap_new_std__string( mSoap, -1 );
  fe->value->append( gwRecordIDFromIcal.latin1() );
  fe->custom = 0;
  fe->date = 0;
  getItemRequest.filter->element = fe;
  getItemRequest.container = &calendarFolderID;
  getItemRequest.items = 0;
  getItemRequest.count = 1;

  mSoap->header->ngwt__session = mSession;
  int result = soap_call___ngw__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                                   &getItemRequest, &getItemResponse );
  if ( !checkResponse( result, getItemResponse.status ) ) return std::string();

  std::vector<class ngwt__Item * > *items = &getItemResponse.items->item;
  if ( items ) {
    std::vector<class ngwt__Item * >::const_iterator it = items->begin();
    if ( it != items->end() )
    {
      ngwt__Item * item = *it;
      fullItemID = *item->id;
    }
  }

  if ( !fullItemID.empty() )
  {
    kdDebug() << " obtained full item id " << fullItemID.c_str() << endl;
  }
  return fullItemID;
}

bool GroupwiseServer::acceptIncidence( KCal::Incidence *incidence )
{
  kdDebug() << "GroupwiseServer::acceptIncidence() " << incidence->schedulingID() << " : " << incidence->summary()             << endl;
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::acceptIncidence(): no session." << endl;
    return false;
  }

  GWConverter conv( mSoap );

  QString qGwUid = incidence->customProperty( "GWRESOURCE", "UID" );
  std::string gwUID;

  if ( qGwUid.isEmpty() )
  {
    QString gwRecordIDFromIcal = incidence->nonKDECustomProperty( "X-GWRECORDID" );
    // we need to do a getItem to get the item's complete ID, including the container portion
    // this is only necessary because the Ical GWRECORDID property is incomplete
    gwUID = getFullIDFor( gwRecordIDFromIcal );
  }
  else
    gwUID = qGwUid.latin1();

  if ( gwUID.empty() )
  {
    kdError() << "GroupwiseServer::declineIncidence(): no GroupWise item ID." << endl;
    return false;
  }

  _ngwm__acceptRequest request;
  _ngwm__acceptResponse response;

  request.comment = 0;
  request.acceptLevel = 0;
  request.recurrenceAllInstances = 0; /*FIXME: This should be the recurrence key for recurring events */
  request.items = soap_new_ngwt__ItemRefList( mSoap, -1 );
/*  std::string acceptedItem;
  acceptedItem.append( gwRecordID.utf8() );*/
  request.items->item.push_back( gwUID );

  mSoap->header->ngwt__session = mSession;
  int result = soap_call___ngw__acceptRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;

  return true;
}

bool GroupwiseServer::declineIncidence( KCal::Incidence *incidence )
{
  kdDebug() << "GroupwiseServer::declineIncidence() " << incidence->schedulingID() << " : " << incidence->summary()             << endl;
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::declineIncidence(): no session." << endl;
    return false;
  }

  GWConverter conv( mSoap );

  std::string gwUID = incidence->customProperty( "GWRESOURCE", "UID" ).latin1();

  if ( gwUID.empty() )
  {
    QString gwRecordIDFromIcal = incidence->nonKDECustomProperty( "X-GWRECORDID" );
    // we need to do a getItem to get the item's complete ID, including the container portion
    // this is only necessary because the Ical GWRECORDID property is incomplete
    gwUID = getFullIDFor( gwRecordIDFromIcal );
  }

  if ( gwUID.empty() )
  {
    kdError() << "GroupwiseServer::declineIncidence(): no GroupWise item ID." << endl;
    return false;
  }

  _ngwm__declineRequest request;
  _ngwm__declineResponse response;

  request.comment = 0;
  request.recurrenceAllInstances = 0; /*FIXME: This should be the recurrence key for recurring events */
  request.items = soap_new_ngwt__ItemRefList( mSoap, -1 );
/*  std::string acceptedItem;
  acceptedItem.append( gwRecordID.utf8() );*/
  request.items->item.push_back( gwUID );

  mSoap->header->ngwt__session = mSession;
  int result = soap_call___ngw__declineRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;

  return true;
}


bool GroupwiseServer::addIncidence( KCal::Incidence *incidence,
  KCal::ResourceCached * )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::addIncidence(): no session." << endl;
    return false;
  }

  kdDebug() << "GroupwiseServer::addIncidence() " << incidence->summary()
            << endl;

  QString gwRecordIDFromIcal = incidence->nonKDECustomProperty( "X-GWRECORDID" );
  if( !gwRecordIDFromIcal.isEmpty() || !incidence->customProperty( "GWRESOURCE", "UID" ).isEmpty() ) {
    kdDebug() << "Incidence has GroupWise ID already: (" << gwRecordIDFromIcal << "ical|" << incidence->customProperty( "GWRESOURCE", "UID" ) <<  "soap) and organizer : " << incidence->organizer().email() << endl;
     return acceptIncidence( incidence );
  }
  else
    kdDebug() << "Incidence has no scheduling ID." << endl;

  IncidenceConverter converter( mSoap );
  converter.setFrom( mUserName, mUserEmail, mUserUuid );

  incidence->setCustomProperty( "GWRESOURCE", "CONTAINER",
                                converter.stringToQString( mCalendarFolder ) );

  ngwt__Item *item;
  if ( incidence->type() == "Event" ) {
    item = converter.convertToAppointment( static_cast<KCal::Event *>( incidence ) );
  } else if ( incidence->type() == "Todo" ) {
    item = converter.convertToTask( static_cast<KCal::Todo *>( incidence ) );
  } else if ( incidence->type() == "Journal" ) {
    item = converter.convertToNote( static_cast<KCal::Journal *>( incidence ) );;
  } else {
    kdError() << "KCal::GroupwiseServer::addIncidence(): Unknown type: "
              << incidence->type() << endl;
    return false;
  }

  _ngwm__sendItemRequest request;
  request.item = item;

  _ngwm__sendItemResponse response;
  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__sendItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;

//  kdDebug() << "RESPONDED UID: " << response.id.c_str() << endl;

  // what if this returns multiple IDs - does a single recurring incidence create multiple ids on the server?
  if ( response.id.size() == 1 )
  {
  std::string firstId = *(response.id.begin() );
  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                QString::fromUtf8( firstId.c_str() ) );
  }
  return true;
}

bool GroupwiseServer::changeIncidence( KCal::Incidence *incidence )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::changeIncidence(): no session." << endl;
    return false;
  }

  kdDebug() << "GroupwiseServer::changeIncidence() " << incidence->summary()
            << endl;

  bool success = true;
  bool todoCompletionChanged = false;

  IncidenceConverter converter( mSoap );
  converter.setFrom( mUserName, mUserEmail, mUserUuid );

  incidence->setCustomProperty( "GWRESOURCE", "CONTAINER",
                                converter.stringToQString( mCalendarFolder ) );

  ngwt__Item *item;
  if ( incidence->type() == "Event" ) {
    item = converter.convertToAppointment( static_cast<KCal::Event *>( incidence ) );
  } else if ( incidence->type() == "Todo" ) {
    item = converter.convertToTask( static_cast<KCal::Todo *>( incidence ) );
  } else if ( incidence->type() == "Journal" ) {
    item = converter.convertToNote( static_cast<KCal::Journal *>( incidence ) );;
  } else {
    kdError() << "KCal::GroupwiseServer::changeIncidence(): Unknown type: "
        << incidence->type() << endl;
    return false;
  }

  if ( iAmTheOrganizer( incidence ) )
  {
    if ( incidence->attendeeCount() > 0 ) {
      kdDebug() << "GroupwiseServer::changeIncidence() - retracting old incidence " << endl;
      if ( !retractRequest( incidence, DueToResend ) ) {
        kdDebug() << "GroupwiseServer::changeIncidence() - retracting failed." << endl;
        return false;
      }
      kdDebug() << "GroupwiseServer::changeIncidence() - adding new meeting with attendees" << endl;
      if ( !addIncidence( incidence, 0 ) ) {
        kdDebug() << "GroupwiseServer::changeIncidence() - adding failed." << endl;
        return false;
      }
      return true;
    }
  }
  else  // If I am not the organizer restrict my changes to accept or decline requests or task completion
  {
    // find myself as attendee.
    GWConverter conv( mSoap );
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      if ( conv.emailsMatch( (*it)->email(), mUserEmail ) ) {
        if ( (*it)->status() == KCal::Attendee::Accepted )
          success &= acceptIncidence( incidence );
        else if ( (*it)->status() == KCal::Attendee::Declined )
          success &= declineIncidence( incidence );
        break;
      }
    }

      // task completion
    if ( incidence->type() == "Todo" )
    {
      KCal::Todo * todo = static_cast<KCal::Todo *>( incidence );
      success &= setCompleted( todo );
    //assume nothing else to change
    }
    return success;

    // if we are attending, but not the organiser, and we have not accepted or declined, there's nothing else to do.
    return true;
  }

  _ngwm__modifyItemRequest request;
  if ( !item->id ) {
    kdError() << "Missing incidence id" << endl;
  } else {
    request.id = *item->id;
  }
  request.updates = soap_new_ngwt__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = item;
  request.notification = 0;
  request.recurrenceAllInstances = 0;
  _ngwm__modifyItemResponse response;
  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__modifyItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );

  success &= checkResponse( result, response.status );

  // task completion after modify
  if ( incidence->type() == "Todo" )
  {
    KCal::Todo * todo = static_cast<KCal::Todo *>( incidence );
    success &= setCompleted( todo );
  }

  return success;
}

bool GroupwiseServer::checkResponse( int result, ngwt__Status *status )
{
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  } else {
    kdDebug() << "SOAP call succeeded" << endl;
  }
  if ( status && status->code != 0 ) {
    QString msg = "SOAP Response Status: " + QString::number( status->code );
    if ( status->description ) {
      msg += " ";
      msg += status->description->c_str();
      mErrorText = status->description->c_str();
    }
    kdError() << msg << endl;
    return false;
  } else {
    return true;
  }
}

bool GroupwiseServer::deleteIncidence( KCal::Incidence *incidence )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::deleteIncidence(): no session." << endl;
    return false;
  }

  kdDebug() << "GroupwiseServer::deleteIncidence(): " << incidence->summary()
            << endl;

  // decline if necessary on the server
  QString gwRecordIDFromIcal = incidence->nonKDECustomProperty( "X-GWRECORDID" );

  // debug contents of message custom properties
  kdDebug() << "incidence custom properties BEGIN" << endl;
  typedef QMap<QCString, QString> PropMap;
  PropMap customs = incidence->customProperties();
  PropMap::Iterator it;
  for ( it = customs.begin(); it != customs.end(); ++it )
    kdDebug() << "key: " << it.key() << ", data: " << it.data() << endl;
  kdDebug() << "incidence custom properties END" << endl;

  if ( incidence->attendeeCount() > 0 ) {
    kdDebug() << "Incidence has GroupWise ID already: (" << gwRecordIDFromIcal << "ical|" << incidence->customProperty( "GWRESOURCE", "UID" ) <<  "soap) and organizer : " << incidence->organizer().email() << endl;
    return declineIncidence( incidence );
  }


#if 0
  kdDebug() << "UID: " << incidence->customProperty( "GWRESOURCE", "UID" )
            << endl;
  kdDebug() << "CONTAINER: " << incidence->customProperty( "GWRESOURCE", "CONTAINER" )
            << endl;
#endif

  if ( incidence->customProperty( "GWRESOURCE", "UID" ).isEmpty() ||
       incidence->customProperty( "GWRESOURCE", "CONTAINER" ).isEmpty() )
    return false;

  _ngwm__removeItemRequest request;
  _ngwm__removeItemResponse response;
  mSoap->header->ngwt__session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( incidence->customProperty( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( incidence->customProperty( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ngw__removeItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  return checkResponse( result, response.status );
}

bool GroupwiseServer::retractRequest( KCal::Incidence *incidence, RetractCause cause )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::retractRequest(): no session." << endl;
    return false;
  }

  kdDebug() << "GroupwiseServer::retractRequest(): " << incidence->summary()
            << endl;
  IncidenceConverter converter( mSoap );
  converter.setFrom( mUserName, mUserEmail, mUserUuid );

  incidence->setCustomProperty( "GWRESOURCE", "CONTAINER",
								converter.stringToQString( mCalendarFolder ) );

  ngwt__Item *item;
  if ( incidence->type() == "Event" ) {
      item = converter.convertToAppointment( static_cast<KCal::Event *>( incidence ) );
  } else if ( incidence->type() == "Todo" ) {
      item = converter.convertToTask( static_cast<KCal::Todo *>( incidence ) );
  } else if ( incidence->type() == "Journal" ) {
      item = converter.convertToNote( static_cast<KCal::Journal *>( incidence ) );;
  } else {
      kdError() << "KCal::GroupwiseServer::addIncidence(): Unknown type: "
              << incidence->type() << endl;
      return false;
  }

  _ngwm__retractRequest request;
  _ngwm__retractResponse response;
  mSoap->header->ngwt__session = mSession;
  request.items = soap_new_ngwt__ItemRefList( mSoap, 1 );
  request.items->item.push_back( *( item->id ) );
  request.comment = 0;
  request.retractCausedByResend = (bool*)soap_malloc( mSoap, 1 );
  request.retractingAllInstances = (bool*)soap_malloc( mSoap, 1 );
  request.retractCausedByResend = ( cause == DueToResend );
  request.retractingAllInstances = true;
  ngwt__RetractType * rt = new ngwt__RetractType;
  *rt = allMailboxes;
  request.retractType = rt;

  int result = soap_call___ngw__retractRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  delete rt;
  return checkResponse( result, response.status );
}

bool GroupwiseServer::insertAddressee( const QString &addrBookId, KABC::Addressee &addr )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::insertAddressee(): no session." << endl;
    return false;
  }

  ContactConverter converter( mSoap );

  addr.insertCustom( "GWRESOURCE", "CONTAINER", addrBookId );

  ngwt__Contact* contact = converter.convertToContact( addr );

  _ngwm__createItemRequest request;
  request.item = contact;
  request.notification = 0;

  _ngwm__createItemResponse response;
  mSoap->header->ngwt__session = mSession;


  int result = soap_call___ngw__createItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;

  addr.insertCustom( "GWRESOURCE", "UID", QString::fromUtf8( response.id.front().c_str() ) );
  addr.setChanged( false );

  return true;
}

bool GroupwiseServer::changeAddressee( const KABC::Addressee &addr )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::changeAddressee(): no session." << endl;
    return false;
  }

  ContactConverter converter( mSoap );

  ngwt__Contact* contact = converter.convertToContact( addr );

  _ngwm__modifyItemRequest request;
  if ( !contact->id ) {
    kdError() << "Missing addressee id" << endl;
  } else {
    request.id = *contact->id;
  }
  request.updates = soap_new_ngwt__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = contact;
  request.notification = 0;
  request.recurrenceAllInstances = 0;

  _ngwm__modifyItemResponse response;
  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__modifyItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  return checkResponse( result, response.status );
}

bool GroupwiseServer::removeAddressee( const KABC::Addressee &addr )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::removeAddressee(): no session." << endl;
    return false;
  }

  if ( addr.custom( "GWRESOURCE", "UID" ).isEmpty() ||
       addr.custom( "GWRESOURCE", "CONTAINER" ).isEmpty() )
    return false;

  _ngwm__removeItemRequest request;
  _ngwm__removeItemResponse response;
  mSoap->header->ngwt__session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( addr.custom( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( addr.custom( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ngw__removeItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  return checkResponse( result, response.status );
}

bool GroupwiseServer::readCalendarSynchronous( KCal::Calendar *cal )
{
  kdDebug() << "GroupwiseServer::readCalendar()" << endl;

  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::readCalendar(): no session." << endl;
    return false;
  }

  ReadCalendarJob *job = new ReadCalendarJob( this, mSoap, mUrl, mSession );
  job->setCalendarFolder( &mCalendarFolder );
  job->setChecklistFolder( &mCheckListFolder );
  job->setCalendar( cal );

  job->run();

  return true;
}

bool GroupwiseServer::readFreeBusy( const QString &email,
  const QDate &start, const QDate &end, KCal::FreeBusy *freeBusy )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::readFreeBusy(): no session." << endl;
    return false;
  }

  kdDebug() << "GroupwiseServer::readFreeBusy()" << endl;

  GWConverter conv( mSoap );

  // Setup input data
  ngwt__NameAndEmail user;
  user.displayName = 0;
  user.uuid = 0;
  user.email = conv.qStringToString( email );

  std::vector<class ngwt__NameAndEmail * > users;
  users.push_back( &user );

  ngwt__FreeBusyUserList userList;
  userList.user = users;

  // Start session
  _ngwm__startFreeBusySessionRequest startSessionRequest;
  startSessionRequest.users = &userList;
  startSessionRequest.startDate = conv.qDateToChar( start );
  startSessionRequest.endDate = conv.qDateToChar( end );

  _ngwm__startFreeBusySessionResponse startSessionResponse;

  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__startFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &startSessionRequest, &startSessionResponse );
  if ( !checkResponse( result, startSessionResponse.status ) ) return false;

  int fbSessionId = *startSessionResponse.freeBusySessionId;

  kdDebug() << "Free/Busy session ID: " << fbSessionId << endl;


  // Get free/busy data
  _ngwm__getFreeBusyRequest getFreeBusyRequest;
  getFreeBusyRequest.freeBusySessionId = QString::number( fbSessionId ).utf8();

  _ngwm__getFreeBusyResponse getFreeBusyResponse;

  

  bool done = false;

  do {
    mSoap->header->ngwt__session = mSession;
    result = soap_call___ngw__getFreeBusyRequest( mSoap,
      mUrl.latin1(), NULL, &getFreeBusyRequest, &getFreeBusyResponse );
    if ( !checkResponse( result, getFreeBusyResponse.status ) ) {
      return false;
    }

    ngwt__FreeBusyStats *stats = getFreeBusyResponse.freeBusyStats;
    if ( !stats || stats->outstanding == 0 ) done = true;

    if ( !stats ) {
      kdDebug() << "NO STATS!" << endl;
    } else {
      kdDebug() << "COUNT: " << stats->responded << " " << stats->outstanding
        << " " << stats->total << endl; 
    }

    std::vector<class ngwt__FreeBusyInfo *> *infos = 0;
    if ( getFreeBusyResponse.freeBusyInfo ) infos =
      &getFreeBusyResponse.freeBusyInfo->user;

    if ( infos ) {
      std::vector<class ngwt__FreeBusyInfo *>::const_iterator it;
      for( it = infos->begin(); it != infos->end(); ++it ) {
        std::vector<class ngwt__FreeBusyBlock *> *blocks = 0;
        if ( (*it)->blocks ) blocks = &(*it)->blocks->block;
        if ( blocks ) {
          std::vector<class ngwt__FreeBusyBlock *>::const_iterator it2;
          for( it2 = blocks->begin(); it2 != blocks->end(); ++it2 ) {
            QDateTime blockStart = conv.charToQDateTime( (*it2)->startDate );
            QDateTime blockEnd = conv.charToQDateTime( (*it2)->endDate );
            ngwt__AcceptLevel acceptLevel = *(*it2)->acceptLevel;

            /* TODO: show Free/Busy subject in diagram - we need to support these as people use it for checking others' calendars */ 
/*            if ( (*it2)->subject )
              std::string subject = *(*it2)->subject;*/
  //          kdDebug() << "BLOCK Subject: " << subject.c_str() << endl;

            if ( acceptLevel == Busy || acceptLevel == OutOfOffice ) {
              freeBusy->addPeriod( blockStart, blockEnd );
            }
          }
        }
      }
    }
  } while ( !done );

  // Close session
  _ngwm__closeFreeBusySessionRequest closeSessionRequest;
  closeSessionRequest.freeBusySessionId = fbSessionId;

  _ngwm__closeFreeBusySessionResponse closeSessionResponse;

  mSoap->header->ngwt__session = mSession;

  result = soap_call___ngw__closeFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &closeSessionRequest, &closeSessionResponse );
  if ( !checkResponse( result, closeSessionResponse.status ) ) return false;

  return true;
}

void GroupwiseServer::slotSslError()
{
  kdDebug() << "********************** SSL ERROR" << endl;

  mErrorText = i18n("SSL Error");
}

void GroupwiseServer::emitReadAddressBookTotalSize( int s )
{
  emit readAddressBookTotalSize( s );
}

void GroupwiseServer::emitReadAddressBookProcessedSize( int s )
{
  emit readAddressBookProcessedSize( s );
}

void GroupwiseServer::emitErrorMessage( const QString & msg, bool fatal )
{
  emit errorMessage( msg, fatal );
}

void GroupwiseServer::emitGotAddressees( const KABC::Addressee::List addressees )
{
  emit gotAddressees( addressees );
}

void GroupwiseServer::log( const QString &prefix, const char *s, size_t n )
{
  if ( mLogFile.isEmpty() ) return;

  kdDebug() << "GroupwiseServer::log() " << prefix << " " << n << " bytes"
    << endl;

  QString log = mLogFile + "_" + QString::number( getpid() ) +
    "_" + prefix + ".log";
  QFile f( log );
  if ( !f.open( IO_WriteOnly | IO_Append ) ) {
    kdError() << "Unable to open log file '" << log << "'" << endl;
  } else {
    uint written = 0;
    while ( written < n ) {
      kdDebug() << "written: " << written << endl;
      int w = f.writeBlock( s + written, n - written );
      kdDebug() << "w: " << w << endl;
      if ( w < 0 ) {
        kdError() << "Unable to write log '" << log << "'" << endl;
        break;
      }
      written += w;
    }
    f.putch( '\n' );
    f.close();
  }
}

bool GroupwiseServer::readUserSettings( ngwt__Settings *&returnedSettings )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::userSettings(): no session." << endl;
    returnedSettings = 0;
    return returnedSettings;
  }

  _ngwm__getSettingsRequest request;

#if 0
  // server doesn't give return any settings keys even if id is a null string
  request.id = soap_new_std__string( mSoap, -1 );
  request.id->append("allowSharedFolders");
#else
  request.id = 0;
#endif

  _ngwm__getSettingsResponse response;
  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__getSettingsRequest( mSoap, mUrl.latin1(), 0, &request, &response );

  if ( !checkResponse( result, response.status ) )
  {
    kdDebug() << "GroupwiseServer::userSettings() - checkResponse() failed" << endl;
    returnedSettings = 0;
    return returnedSettings;
  }

  returnedSettings = response.settings;
  if ( !returnedSettings )
  {
    kdDebug() << "GroupwiseServer::userSettings() - no settings in response. " << endl;
    // debug data pending server fix
    // settings
    returnedSettings = new ngwt__Settings;
    // list of groups
    //returnedSettings->group = new std::vector<ngwt__SettingsGroup * >; // sample group
    ngwt__SettingsGroup * grp = new ngwt__SettingsGroup;
    grp->type = new std::string;
    grp->type->append( "GROUP 1" );
    // list of settings
    //grp->setting = new std::vector<ngwt__Custom * >;
    // 2 sample settings
    ngwt__Custom * setting1 = new ngwt__Custom;
    setting1->field.append("Setting 1");
    setting1->value = new std::string;
    setting1->value->append("Value 1 ");
    setting1->locked = new bool;
    *(setting1->locked) = false;
    ngwt__Custom * setting2 = new ngwt__Custom;
    setting2->field.append("Setting 2");
    setting2->value = new std::string;
    setting2->value->append("Value 2");
    setting2->locked = new bool;
    *(setting2->locked) = true;
    grp->setting.push_back( setting1 );
    grp->setting.push_back( setting2 );
    
    returnedSettings->group.push_back( grp );
  }
  kdDebug() << "GroupwiseServer::userSettings() - done. " << endl;
  
  return true; /** FIXME return false if no settings fetched */
}

bool GroupwiseServer::modifyUserSettings( QMap<QString, QString> & settings  )
{
  kdDebug() << "GroupwiseServer::userSettings()" << endl;

  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::userSettings(): no session." << endl;
    return false;
  }
  _ngwm__modifySettingsRequest request;
  _ngwm__modifySettingsResponse response;
  request.settings = soap_new_ngwt__SettingsList( mSoap, -1 );
  QMap<QString, QString>::Iterator it;
  for ( it = settings.begin(); it != settings.end(); ++it )
  {
    kdDebug() << " creating Custom for " << it.key() << ", " << it.data() << endl;
    ngwt__Custom * custom = soap_new_ngwt__Custom( mSoap, -1 );
    custom->locked = 0;
    custom->field.append( it.key().utf8() );
    custom->value = soap_new_std__string( mSoap, -1 );
    custom->value->append( it.data().utf8() );
    request.settings->setting.push_back( custom );

  }

  mSoap->header->ngwt__session = mSession;

  int result = soap_call___ngw__modifySettingsRequest( mSoap, mUrl.latin1(), 0, &request, &response );
  if ( !checkResponse( result, response.status ) )
  {
    kdDebug() << "GroupwiseServer::modifyUserSettings() - checkResponse() failed" << endl;
    return false;
  }
  kdError() << "GroupwiseServer::userSettings() - success" << endl;
  return true;
}

bool GroupwiseServer::iAmTheOrganizer( KCal::Incidence * incidence )
{
  return ( incidence->organizer().email() == mUserEmail );
}

bool GroupwiseServer::setCompleted( KCal::Todo * todo )
{
  if ( todo )
  {
    GWConverter conv( mSoap );
    QString id = todo->customProperty( "GWRESOURCE", "UID" );
    ngwt__ItemRefList * items = soap_new_ngwt__ItemRefList( mSoap, 1 );
    items->item.push_back( *( conv.qStringToString( id ) ) );
    if ( todo->isCompleted() )
    {
      _ngwm__completeRequest request;
      _ngwm__completeResponse response;
      mSoap->header->ngwt__session = mSession;
      request.items = items;
      int result = soap_call___ngw__completeRequest( mSoap, mUrl.latin1(), 0,
          &request, &response );
      return checkResponse( result, response.status );
    }
    else
    {
      _ngwm__uncompleteRequest request;
      _ngwm__uncompleteResponse response;
      mSoap->header->ngwt__session = mSession;
      request.items = items;
      int result = soap_call___ngw__uncompleteRequest( mSoap, mUrl.latin1(), 0,
          &request, &response );
      return checkResponse( result, response.status );
    }
  }
  return false;
}

#include "groupwiseserver.moc"
