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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// The namespace mapping table is required and associates namespace prefixes
// with namespace names:
#include "GroupWiseBinding.nsmap"

#include <libkcal/calendar.h>
#include <libkcal/incidence.h>

#include <kabc/addressee.h>

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
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

#include <stdlib.h>
#include <stdio.h>

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

int GroupwiseServer::gSoapOpen( struct soap *, const char *,
  const char *host, int port )
{
  kdDebug() << "GroupwiseServer::gSoapOpen()" << endl;

  if ( m_sock ) {
    kdError() << "m_sock non-null: " << (void*)m_sock << endl;
    delete m_sock;
  }

  if ( mSSL ) {
    kdDebug() << "Creating KSSLSocket()" << endl;
    m_sock = new KSSLSocket();
    connect( m_sock, SIGNAL( sslFailure() ), SLOT( slotSslError() ) );
  } else {
    m_sock = new KExtendedSocket();
  }
  mError = QString::null;

  m_sock->reset();
  m_sock->setBlockingMode( false );
  m_sock->setSocketFlags( KExtendedSocket::inetSocket );

  m_sock->setAddress( host, port );
  m_sock->lookup();
  int rc = m_sock->connect();
  if ( rc != 0 ) {
    kdError() << "gSoapOpen: connect failed " << rc << endl;
    mError = i18n("Connect failed: %1.").arg( rc );
    if ( rc == -1 ) perror( 0 );
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
  kdDebug() << "GroupwiseServer::gSoapClose()" << endl;

  delete m_sock;
  m_sock = 0;

#if 0
   m_sock->close();
   m_sock->reset();
#endif
   return SOAP_OK;
}

int GroupwiseServer::gSoapSendCallback( struct soap *, const char *s, size_t n )
{
  kdDebug() << "GroupwiseServer::gSoapSendCallback()" << endl;

  if ( !m_sock ) {
    kdError() << "no open connection" << endl;
    return SOAP_TCP_ERROR;
  }
  if ( !mError.isEmpty() ) {
    kdError() << "SSL is in error state." << endl;
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
      return SOAP_TCP_ERROR;
    }
    n -= ret;
  }

  if ( n !=0 ) {
    kdError() << "Send failed: " << strerror( m_sock->systemError() )
      << " " << m_sock->socketStatus() << " " << m_sock->fd() << endl;
  }

  m_sock->flush();

  return SOAP_OK;
}

size_t GroupwiseServer::gSoapReceiveCallback( struct soap *soap, char *s,
  size_t n )
{
  kdDebug() << "GroupwiseServer::gSoapReceiveCallback()" << endl;

  if ( !m_sock ) {
    kdError() << "no open connection" << endl;
    soap->error = SOAP_FAULT;
    return 0;
  }
  if ( !mError.isEmpty() ) {
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
      qDebug("kioReceiveCallback return %d", ret);
    }
    log( "RECV", s, ret );
  }

  return ret;
}

GroupwiseServer::GroupwiseServer( const QString &url, const QString &user,
                                  const QString &password, QObject *parent )
  : QObject( parent, "GroupwiseServer" ),
    mUrl( url ), mUser( user ), mPassword( password ),
    mSSL( url.left(6)=="https:" ), m_sock( 0 )
{
  mSoap = new soap;

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
  _ns1__loginResponse loginResp;
  _ns1__loginRequest loginReq;
  loginReq.language = 0;
  loginReq.version = 0;

  GWConverter conv( mSoap );

  ns1__PlainText pt;

  pt.username = mUser.utf8();
  pt.password = conv.qStringToString( mPassword );
  loginReq.auth = &pt;
  mSoap->userid = strdup( mUser.utf8() );
  mSoap->passwd = strdup( mPassword.utf8() );

  mSession = "";

  kdDebug() << "GroupwiseServer::login() URL: " << mUrl << endl;

  int result = 1, maxTries = 3;

  while ( --maxTries && result ) {
    result = soap_call___ns1__loginRequest( mSoap, mUrl.latin1(), NULL,
      &loginReq, &loginResp );
  }

  if ( !checkResponse( result, loginResp.status ) ) return false;

  mSession = loginResp.session;
  mSoap->header = new( SOAP_ENV__Header );

  mUserName = "";
  mUserEmail = "";
  mUserUuid = "";

  ns1__UserInfo *userinfo = loginResp.UserInfo;
  if ( userinfo ) {
    kdDebug() << "HAS USERINFO" << endl;
    mUserName = conv.stringToQString( userinfo->name );
    if ( userinfo->email ) mUserEmail = conv.stringToQString( userinfo->email );
    if ( userinfo->uuid ) mUserUuid = conv.stringToQString( userinfo->uuid );
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

  _ns1__getCategoryListResponse catListResp;
  mSoap->header->session = mSession;
  int result = soap_call___ns1__getCategoryListRequest( mSoap, mUrl.latin1(),
    0, "", &catListResp);
  if ( !checkResponse( result, catListResp.status ) ) return false;

  if ( catListResp.categories ) {
    std::vector<class ns1__Category * > *categories;
    categories = catListResp.categories->category;
    std::vector<class ns1__Category * >::const_iterator it;
    for( it = categories->begin(); it != categories->end(); ++it ) {
//      cout << "CATEGORY" << endl;
      dumpItem( *it );
    }
  }

  return true;
}

bool GroupwiseServer::dumpData()
{
  mSoap->header->session = mSession;
  _ns1__getAddressBookListResponse addressBookListResponse;
  soap_call___ns1__getAddressBookListRequest( mSoap, mUrl.latin1(),
                                              NULL, "", &addressBookListResponse );
  soap_print_fault(mSoap, stderr);

  if ( addressBookListResponse.books ) {
    std::vector<class ns1__AddressBook * > *addressBooks = addressBookListResponse.books->book;
    std::vector<class ns1__AddressBook * >::const_iterator it;
    for( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      ns1__AddressBook *book = *it;
      if ( book->description ) {
        cout << "ADDRESSBOOK: description: " << book->description->c_str() << endl;
      }
      if ( book->id ) {
        cout << "ADDRESSBOOK: id: " << book->id->c_str() << endl;
      }
      if ( book->name ) {
        cout << "ADDRESSBOOK: name: " << book->name->c_str() << endl;
      }

      _ns1__getItemsRequest itemsRequest;
      if ( !book->id ) {
        kdError() << "Missing book id" << endl;
      } else {
        itemsRequest.container = *(book->id);
      }
      itemsRequest.filter = 0;
      itemsRequest.items = 0;
//      itemsRequest.count = -1;

      mSoap->header->session = mSession;
      _ns1__getItemsResponse itemsResponse;
      soap_call___ns1__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                        &itemsRequest,
                                        &itemsResponse );

      std::vector<class ns1__Item * > *items = itemsResponse.items->item;
      if ( items ) {
        std::vector<class ns1__Item * >::const_iterator it2;
        for( it2 = items->begin(); it2 != items->end(); ++it2 ) {
          cout << "ITEM" << endl;
          dumpItem( *it2 );

          if ( true ) {
            _ns1__getItemRequest itemRequest;
            if ( !(*it2)->id ) {
              kdError() << "Missing item id" << endl;
            } else {
              itemRequest.id = *( (*it2)->id );
            }
            itemRequest.view = 0;

            mSoap->header->session = mSession;
            _ns1__getItemResponse itemResponse;
            soap_call___ns1__getItemRequest( mSoap, mUrl.latin1(), 0,
                                             &itemRequest,
                                             &itemResponse );

            ns1__Item *item = itemResponse.item;
            ns1__Contact *contact = dynamic_cast<ns1__Contact *>( item );
            if ( !contact ) {
              cerr << "Cast failed." << endl;
            } else {
              cout << "Cast succeeded." << endl;
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
  mSoap->header->session = mSession;
  _ns1__getFolderListRequest folderListReq;
  folderListReq.parent = "folders";
  folderListReq.recurse = true;
  _ns1__getFolderListResponse folderListRes;
  soap_call___ns1__getFolderListRequest( mSoap, mUrl.latin1(), 0,
                                         &folderListReq,
                                         &folderListRes );

  if ( folderListRes.folders ) {
    std::vector<class ns1__Folder * > *folders = folderListRes.folders->folder;
    if ( folders ) {
      std::vector<class ns1__Folder * >::const_iterator it;
      for ( it = folders->begin(); it != folders->end(); ++it ) {
        cout << "FOLDER" << endl;
        dumpFolder( *it );
#if 1
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
  _ns1__getItemsRequest itemsRequest;

  itemsRequest.container = id;
  itemsRequest.view = "recipients message recipientStatus";

  itemsRequest.filter = 0;
  itemsRequest.items = 0;
//  itemsRequest.count = 5;

  mSoap->header->session = mSession;
  _ns1__getItemsResponse itemsResponse;
  soap_call___ns1__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  soap_print_fault(mSoap, stderr);

  std::vector<class ns1__Item * > *items = itemsResponse.items->item;

  if ( items ) {
    std::vector<class ns1__Item * >::const_iterator it;
    for( it = items->begin(); it != items->end(); ++it ) {
      if ( (*it)->type ) {
        kdDebug() << "ITEM type '" << (*it)->type->c_str() << "'" << endl;
      } else {
        kdDebug() << "ITEM no type" << endl;
      }
      
      ns1__Appointment *a = dynamic_cast<ns1__Appointment *>( *it );
      if ( !a ) {
        cerr << "Appointment cast failed." << endl;
      } else {
        cout << "CALENDAR ITEM" << endl;
        dumpAppointment( a );
      }
      ns1__Task *t = dynamic_cast<ns1__Task *>( *it );
      if ( !t ) {
        cerr << "Task cast failed." << endl;
      } else {
        cout << "TASK" << endl;
        dumpTask( t );
      }
    }
  }
}

void GroupwiseServer::dumpMail( ns1__Mail *m )
{
  dumpItem( m );
  cout << "  SUBJECT: " << m->subject << endl;
}

void GroupwiseServer::dumpTask( ns1__Task *t )
{
  dumpMail( t );
  if ( t->completed ) {
    cout << "  COMPLETED: " << ( t->completed ? "true" : "false" ) << endl; 
  }
}

void GroupwiseServer::dumpAppointment( ns1__Appointment *a )
{
  dumpMail( a );
  cout << "  START DATE: " << a->startDate << endl;
  cout << "  END DATE: " << a->endDate << endl;
  if ( a->allDayEvent ) {
    cout << "  ALL DAY: " << ( a->allDayEvent ? "true" : "false" ) << endl;
  }
}

void GroupwiseServer::dumpFolder( ns1__Folder *f )
{
  dumpItem( f );
  cout << "  PARENT: " << f->parent.c_str() << endl;
  if ( f->description ) {
    cout << "  DESCRIPTION: " << f->description->c_str() << endl;
  }
  // FIXME: More fields
//	int *count;
//	bool *hasUnread;
//	int *unreadCount;
//	unsigned long sequence;
//	std::string *settings;
//	bool *hasSubfolders;
//	ns1__SharedFolderNotification *notification;
}

void GroupwiseServer::dumpItem( ns1__Item *i )
{
  if ( !i ) return;
  if ( i->id ) {
    cout << "  ID: " << i->id->c_str() << endl;
  }
  if ( i->name ) {
    cout << "  NAME: " << i->name->c_str() << endl;
  }
  cout << "  VERSION: " << i->version << endl;
  cout << "  MODIFIED: " << i->modified << endl;
  if ( i->changes ) cout << "  HASCHANGES" << endl;
  if ( i->type ) {
    cout << "  TYPE: " << i->type->c_str() << endl;
  }
}

bool GroupwiseServer::logout()
{
  // FIXME: Send logoutRequest

  soap_end( mSoap );
  soap_done( mSoap );

  delete mSoap->header;
  mSoap->header = 0;

  return true;
}

bool GroupwiseServer::getDelta()
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::getDelta(): no session." << endl;
    return false;
  }

  _ns1__getDeltaRequest deltaRequest;
  deltaRequest.AddressBookItem = 0;
  deltaRequest.Appointment = 0;
  deltaRequest.CalendarItem = new string;
  deltaRequest.Contact = 0;
  deltaRequest.Folder = 0;
  deltaRequest.Group = 0;
  deltaRequest.Item = 0;
  deltaRequest.Mail = 0;
  deltaRequest.Note = 0;
  deltaRequest.PhoneMessage = 0;
  deltaRequest.Task = 0;
  
  mSoap->header->session = mSession;
  _ns1__getDeltaResponse deltaResponse;
  int result = soap_call___ns1__getDeltaRequest( mSoap, mUrl.latin1(), 0,
    &deltaRequest, &deltaResponse );
  return checkResponse( result, deltaResponse.status );
}

GroupWise::AddressBook::List GroupwiseServer::addressBookList()
{
  GroupWise::AddressBook::List books;

  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::addressBookList(): no session." << endl;
    return books;
  }

  mSoap->header->session = mSession;
  _ns1__getAddressBookListResponse addressBookListResponse;
  int result = soap_call___ns1__getAddressBookListRequest( mSoap, mUrl.latin1(),
    NULL, "", &addressBookListResponse );
  if ( !checkResponse( result, addressBookListResponse.status ) ) {
    return books;
  }

  if ( addressBookListResponse.books ) {
    std::vector<class ns1__AddressBook * > *addressBooks = addressBookListResponse.books->book;
    std::vector<class ns1__AddressBook * >::const_iterator it;
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

bool GroupwiseServer::readAddressBooksSynchronous( const QStringList &addrBookIds,
  KABC::ResourceCached *resource )
{
  if ( mSession.empty() ) {
    kdError() << "GroupwiseServer::readAddressBooks(): no session." << endl;
    return false;
  }

  ReadAddressBooksJob *job = new ReadAddressBooksJob( this, mSoap,
    mUrl, mSession );
  job->setAddressBookIds( addrBookIds );
  job->setResource( resource );

  job->run();

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

  IncidenceConverter converter( mSoap );
  converter.setFrom( mUserName, mUserEmail, mUserUuid );

  incidence->setCustomProperty( "GWRESOURCE", "CONTAINER",
                                converter.stringToQString( mCalendarFolder ) );

  ns1__Item *item;
  if ( incidence->type() == "Event" ) {
    item = converter.convertToAppointment( static_cast<KCal::Event *>( incidence ) );
  } else if ( incidence->type() == "Todo" ) {
    item = converter.convertToTask( static_cast<KCal::Todo *>( incidence ) );
  } else {
    kdError() << "KCal::GroupwiseServer::addIncidence(): Unknown type: "
              << incidence->type() << endl;
    return false;
  }

  _ns1__sendItemRequest request;
  request.item = item;

  _ns1__sendItemResponse response;
  mSoap->header->session = mSession;

  int result = soap_call___ns1__sendItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;

//  kdDebug() << "RESPONDED UID: " << response.id.c_str() << endl;

  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                QString::fromUtf8( response.id.c_str() ) );

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

  if ( incidence->attendeeCount() > 0 ) {
    if ( !deleteIncidence( incidence ) ) return false;
    if ( !addIncidence( incidence, 0 ) ) return false;
    return true;
  }

  IncidenceConverter converter( mSoap );
  converter.setFrom( mUserName, mUserEmail, mUserUuid );

  incidence->setCustomProperty( "GWRESOURCE", "CONTAINER",
                                converter.stringToQString( mCalendarFolder ) );

  ns1__Item *item;
  if ( incidence->type() == "Event" ) {
    item = converter.convertToAppointment( static_cast<KCal::Event *>( incidence ) );
  } else if ( incidence->type() == "Todo" ) {
    item = converter.convertToTask( static_cast<KCal::Todo *>( incidence ) );
  } else {
    kdError() << "KCal::GroupwiseServer::changeIncidence(): Unknown type: "
              << incidence->type() << endl;
    return false;
  }

  _ns1__modifyItemRequest request;
  if ( !item->id ) {
    kdError() << "Missing incidence id" << endl;
  } else {
    request.id = *item->id;
  }
  request.updates = soap_new_ns1__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = item;

  _ns1__modifyItemResponse response;
  mSoap->header->session = mSession;

  int result = soap_call___ns1__modifyItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  return checkResponse( result, response.status );
}

bool GroupwiseServer::checkResponse( int result, ns1__Status *status )
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

#if 0
  kdDebug() << "UID: " << incidence->customProperty( "GWRESOURCE", "UID" )
            << endl;
  kdDebug() << "CONTAINER: " << incidence->customProperty( "GWRESOURCE", "CONTAINER" )
            << endl;
#endif

  if ( incidence->customProperty( "GWRESOURCE", "UID" ).isEmpty() ||
       incidence->customProperty( "GWRESOURCE", "CONTAINER" ).isEmpty() )
    return false;

  _ns1__removeItemRequest request;
  _ns1__removeItemResponse response;
  mSoap->header->session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( incidence->customProperty( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( incidence->customProperty( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ns1__removeItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
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

  ns1__Contact* contact = converter.convertToContact( addr );

  _ns1__createItemRequest request;
  request.item = contact;

  _ns1__createItemResponse response;
  mSoap->header->session = mSession;


  int result = soap_call___ns1__createItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( !checkResponse( result, response.status ) ) return false;
  
  addr.insertCustom( "GWRESOURCE", "UID", QString::fromUtf8( response.id.c_str() ) );
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

  ns1__Contact* contact = converter.convertToContact( addr );

  _ns1__modifyItemRequest request;
  if ( !contact->id ) {
    kdError() << "Missing addressee id" << endl;
  } else {
    request.id = *contact->id;
  }
  request.updates = soap_new_ns1__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = contact;

  _ns1__modifyItemResponse response;
  mSoap->header->session = mSession;

  int result = soap_call___ns1__modifyItemRequest( mSoap, mUrl.latin1(), 0,
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

  _ns1__removeItemRequest request;
  _ns1__removeItemResponse response;
  mSoap->header->session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( addr.custom( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( addr.custom( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ns1__removeItemRequest( mSoap, mUrl.latin1(), 0,
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

  ReadCalendarJob *job = new ReadCalendarJob( mSoap, mUrl, mSession );
  job->setCalendarFolder( &mCalendarFolder );
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
  ns1__FreeBusyUser user;
  user.email = conv.qStringToString( email );

  std::vector<class ns1__FreeBusyUser * > users;
  users.push_back( &user );

  ns1__FreeBusyUserList userList;
  userList.user = &users;

  // Start session
  _ns1__startFreeBusySessionRequest startSessionRequest;
  startSessionRequest.users = &userList;
  startSessionRequest.startDate = conv.qDateToChar( start );
  startSessionRequest.endDate = conv.qDateToChar( end );
  
  _ns1__startFreeBusySessionResponse startSessionResponse;

  mSoap->header->session = mSession;

  int result = soap_call___ns1__startFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &startSessionRequest, &startSessionResponse );
  if ( !checkResponse( result, startSessionResponse.status ) ) return false;

  int fbSessionId = startSessionResponse.freeBusySessionId;

  kdDebug() << "Free/Busy session ID: " << fbSessionId << endl;


  // Get free/busy data
  _ns1__getFreeBusyRequest getFreeBusyRequest;
  getFreeBusyRequest.freeBusySessionId = QString::number( fbSessionId ).utf8();
  
  _ns1__getFreeBusyResponse getFreeBusyResponse;

  mSoap->header->session = mSession;

  result = soap_call___ns1__getFreeBusyRequest( mSoap,
    mUrl.latin1(), NULL, &getFreeBusyRequest, &getFreeBusyResponse );
  if ( !checkResponse( result, getFreeBusyResponse.status ) ) {
    return false;
  }
 
  std::vector<class ns1__FreeBusyInfo *> *infos = 0;
  if ( getFreeBusyResponse.freeBusyInfo ) infos =
    getFreeBusyResponse.freeBusyInfo->user;

  if ( infos ) {
    std::vector<class ns1__FreeBusyInfo *>::const_iterator it;
    for( it = infos->begin(); it != infos->end(); ++it ) {
      std::vector<class ns1__FreeBusyBlock *> *blocks = 0;
      if ( (*it)->blocks ) blocks = (*it)->blocks->block;
      if ( blocks ) {
        std::vector<class ns1__FreeBusyBlock *>::const_iterator it2;
        for( it2 = blocks->begin(); it2 != blocks->end(); ++it2 ) {
          QDateTime blockStart = conv.charToQDateTime( (*it2)->startDate );
          QDateTime blockEnd = conv.charToQDateTime( (*it2)->endDate );
          ns1__AcceptLevel acceptLevel = (*it2)->acceptLevel;

          std::string subject = (*it2)->subject;
//          kdDebug() << "BLOCK Subject: " << subject.c_str() << endl;

          if ( acceptLevel == Busy || acceptLevel == OutOfOffice ) {
            freeBusy->addPeriod( blockStart, blockEnd );
          }
        }
      }
    }
  }

  // Close session
  _ns1__closeFreeBusySessionRequest closeSessionRequest;
  closeSessionRequest.freeBusySessionId = fbSessionId;
  
  _ns1__closeFreeBusySessionResponse closeSessionResponse;

  mSoap->header->session = mSession;

  result = soap_call___ns1__closeFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &closeSessionRequest, &closeSessionResponse );
  if ( !checkResponse( result, closeSessionResponse.status ) ) return false;

  return true;
}

void GroupwiseServer::slotSslError()
{
  kdDebug() << "********************** SSL ERROR" << endl;

  mError = i18n("SSL Error");
}

void GroupwiseServer::emitReadAddressBookTotalSize( int s )
{
  emit readAddressBookTotalSize( s );
}

void GroupwiseServer::emitReadAddressBookProcessedSize( int s )
{
  emit readAddressBookProcessedSize( s );
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

#include "groupwiseserver.moc"
