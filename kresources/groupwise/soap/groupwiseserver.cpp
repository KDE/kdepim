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
#include "getItemRequestSoapBinding.nsmap"

#include <libkcal/calendar.h>
#include <libkcal/incidence.h>
#include <libkdepim/weaverlogger.h>

#include <kabc/addressee.h>

#include <kdebug.h>

#include "contactconverter.h"
#include "incidenceconverter.h"
#include "kcal_resourcegroupwise.h"
#include "soapH.h"

#include "groupwiseserver.h"

GroupwiseServer::GroupwiseServer( const QString &url, const QString &user,
                                  const QString &password, QObject *parent )
  : QObject( parent, "GroupwiseServer" ),
    mUrl( url ), mUser( user ), mPassword( password )
{
  mSoap = new soap;
  mWeaver = new KPIM::ThreadWeaver::Weaver( this );
  KPIM::ThreadWeaver::WeaverThreadLogger *weaverLogger = new KPIM::ThreadWeaver::WeaverThreadLogger( this );
  weaverLogger->attach( mWeaver );
}

GroupwiseServer::~GroupwiseServer()
{
  delete mWeaver;
  mWeaver = 0;

  delete mSoap;
  mSoap = 0;
}

bool GroupwiseServer::login()
{
  _ns1__loginResponse loginResp;
  _ns1__loginRequest loginReq;
  loginReq.language = 0;
  loginReq.version = 0;

  ns1__PlainText pt;

  soap_init( mSoap );
  pt.username = mUser.latin1();
  pt.password = new string( mPassword.latin1() );
  loginReq.auth = &pt;

  cout << "Login" << endl;

  int result = soap_call___ns2__loginRequest(mSoap, mUrl.latin1(), NULL, &loginReq, &loginResp );

  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  }

  std::cout << "Session ID: " << loginResp.session.c_str() << std::endl;

  mSession = loginResp.session;
  mSoap->header = new( SOAP_ENV__Header );

  return true;
}

bool GroupwiseServer::getCategoryList()
{
  _ns1__getCategoryListResponse catListResp;
  mSoap->header->ns1__session = mSession;
  soap_call___ns3__getCategoryListRequest( mSoap, mUrl.latin1(),
                                           0, "", &catListResp);

  if ( catListResp.categories ) {
    std::vector<class ns1__Category * > *categories;
    categories = catListResp.categories->category;
    std::vector<class ns1__Category * >::const_iterator it;
    for( it = categories->begin(); it != categories->end(); ++it ) {
      cout << "CATEGORY" << endl;
      dumpItem( *it );
    }
  }

  return true;
}

bool GroupwiseServer::dumpData()
{

  mSoap->header->ns1__session = mSession;
  _ns1__getAddressBookListResponse addressBookListResponse;
  soap_call___ns4__getAddressBookListRequest( mSoap, mUrl.latin1(),
                                              NULL, "", &addressBookListResponse );
  soap_print_fault(mSoap, stderr);

  if ( addressBookListResponse.books ) {
    std::vector<class ns1__AddressBook * > *addressBooks = addressBookListResponse.books->book;
    std::vector<class ns1__AddressBook * >::const_iterator it;
    for( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      ns1__AddressBook *book = *it;
      cout << "ADDRESSBOOK: description: " << book->description.c_str() << endl;
      cout << "ADDRESSBOOK: id: " << book->id.c_str() << endl;
      cout << "ADDRESSBOOK: name: " << book->name.c_str() << endl;

      _ns1__getItemsRequest itemsRequest;
      itemsRequest.container = book->id;
      itemsRequest.filter = 0;
      itemsRequest.items = 0;
//      itemsRequest.count = -1;

      mSoap->header->ns1__session = mSession;
      _ns1__getItemsResponse itemsResponse;
      soap_call___ns6__getItemsRequest( mSoap, mUrl.latin1(), 0,
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
            itemRequest.id = (*it2)->id;
            itemRequest.view = 0;

            mSoap->header->ns1__session = mSession;
            _ns1__getItemResponse itemResponse;
            soap_call___ns5__getItemRequest( mSoap, mUrl.latin1(), 0,
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
  mSoap->header->ns1__session = mSession;
  _ns1__getFolderListRequest folderListReq;
  folderListReq.parent = "folders";
  folderListReq.recurse = true;
  _ns1__getFolderListResponse folderListRes;
  soap_call___ns7__getFolderListRequest( mSoap, mUrl.latin1(), 0,
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
          dumpCalendarFolder( (*it)->id );
        }
#else
        dumpCalendarFolder( (*it)->id );   
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

  mSoap->header->ns1__session = mSession;
  _ns1__getItemsResponse itemsResponse;
  soap_call___ns6__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  soap_print_fault(mSoap, stderr);

  std::vector<class ns1__Item * > *items = itemsResponse.items->item;

  if ( items ) {
    std::vector<class ns1__Item * >::const_iterator it;
    for( it = items->begin(); it != items->end(); ++it ) {
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
  cout << "  DESCRIPTION: " << f->description.c_str() << endl;
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
  cout << "  ID: " << i->id.c_str() << endl;
  cout << "  NAME: " << i->name.c_str() << endl;
  cout << "  VERSION: " << i->version << endl;
  cout << "  MODIFIED: " << i->modified << endl;
  if ( i->changes ) cout << "  HASCHANGES" << endl;
  if ( i->type ) {
    cout << "  TYPE: " << i->type->c_str() << endl;
  }
}

bool GroupwiseServer::logout()
{
  soap_end( mSoap );
  soap_done( mSoap );

  delete mSoap->header;
  mSoap->header = 0;

  return true;
}

void GroupwiseServer::getDelta()
{
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
  
  mSoap->header->ns1__session = mSession;
  _ns1__getDeltaResponse deltaResponse;
  soap_call___ns8__getDeltaRequest( mSoap, mUrl.latin1(), 0,
                                    &deltaRequest, &deltaResponse );

}

QMap<QString, QString> GroupwiseServer::addressBookList()
{
  QMap<QString, QString> map;

  mSoap->header->ns1__session = mSession;
  _ns1__getAddressBookListResponse addressBookListResponse;
  soap_call___ns4__getAddressBookListRequest( mSoap, mUrl.latin1(),
                                              NULL, "", &addressBookListResponse );
  soap_print_fault( mSoap, stderr );

  if ( addressBookListResponse.books ) {
    std::vector<class ns1__AddressBook * > *addressBooks = addressBookListResponse.books->book;
    std::vector<class ns1__AddressBook * >::const_iterator it;
    for ( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      map.insert( GWConverter::stringToQString( (*it)->id ), GWConverter::stringToQString( (*it)->name ) );
    }
  }

  return map;
}

bool GroupwiseServer::readAddressBooks( const QStringList &addrBookIds, KABC::ResourceGroupwise *resource )
{
  ReadAddressBooksJob *job = new ReadAddressBooksJob( mSoap, mUrl, mSession, 0 );
  job->setAddressBookIds( addrBookIds );
  job->setResource( resource );

  connect( job, SIGNAL( done() ),
           this, SIGNAL( readAddressBooksFinished() ) );

  mWeaver->enqueue( job );

  return true;
}

bool GroupwiseServer::addIncidence( KCal::Incidence *incidence )
{
  kdDebug() << "GroupwiseServer::addIncidence() " << incidence->summary()
            << endl;

  IncidenceConverter converter( mSoap );

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

  _ns1__createItemRequest request;
  request.item = item;

  _ns1__createItemResponse response;
  mSoap->header->ns1__session = mSession;

  int result = soap_call___ns9__createItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  } else {
    kdDebug() << "SOAP call succeeded" << endl;
  }

  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                QString::fromUtf8( response.id.c_str() ) );

  return true;
}

bool GroupwiseServer::changeIncidence( KCal::Incidence *incidence )
{
  kdDebug() << "GroupwiseServer::changeIncidence() " << incidence->summary()
            << endl;

  IncidenceConverter converter( mSoap );

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

  _ns1__modifyItemRequest request;
  request.id = item->id;
  request.updates = soap_new_ns1__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = item;

  _ns1__modifyItemResponse response;
  mSoap->header->ns1__session = mSession;

  int result = soap_call___ns10__modifyItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  } else {
    kdDebug() << "SOAP call succeeded" << endl;
  }

  return true;
}

bool GroupwiseServer::deleteIncidence( KCal::Incidence *incidence )
{
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
  mSoap->header->ns1__session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( incidence->customProperty( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( incidence->customProperty( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ns12__removeItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  } else {
    kdDebug() << "SOAP call succeeded" << endl;
  }

  return true;
}

bool GroupwiseServer::insertAddressee( const QString &addrBookId, KABC::Addressee &addr )
{
  ContactConverter converter( mSoap );

  addr.insertCustom( "GWRESOURCE", "CONTAINER", addrBookId );

  ns1__Contact* contact = converter.convertToContact( addr );

  _ns1__createItemRequest request;
  request.item = contact;

  _ns1__createItemResponse response;
  mSoap->header->ns1__session = mSession;


  int result = soap_call___ns9__createItemRequest( mSoap, mUrl.latin1(), 0,
                                                   &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  } else {
    addr.insertCustom( "GWRESOURCE", "UID", QString::fromUtf8( response.id.c_str() ) );
    addr.setChanged( false );
  }

  return true;
}

bool GroupwiseServer::changeAddressee( const KABC::Addressee &addr )
{
  ContactConverter converter( mSoap );

  ns1__Contact* contact = converter.convertToContact( addr );

  _ns1__modifyItemRequest request;
  request.id = contact->id;
  request.updates = soap_new_ns1__ItemChanges( mSoap, -1 );
  request.updates->add = 0;
  request.updates->_delete = 0;
  request.updates->update = contact;

  _ns1__modifyItemResponse response;
  mSoap->header->ns1__session = mSession;

  int result = soap_call___ns10__modifyItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  }

  return true;
}

bool GroupwiseServer::removeAddressee( const KABC::Addressee &addr )
{
  if ( addr.custom( "GWRESOURCE", "UID" ).isEmpty() ||
       addr.custom( "GWRESOURCE", "CONTAINER" ).isEmpty() )
    return false;

  _ns1__removeItemRequest request;
  _ns1__removeItemResponse response;
  mSoap->header->ns1__session = mSession;

  GWConverter converter( mSoap );
  request.container = converter.qStringToString( addr.custom( "GWRESOURCE", "CONTAINER" ) );
  request.id = std::string( addr.custom( "GWRESOURCE", "UID" ).utf8() );

  int result = soap_call___ns12__removeItemRequest( mSoap, mUrl.latin1(), 0,
                                                    &request, &response );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  }

  return true;
}

bool GroupwiseServer::readCalendar( KCal::Calendar *calendar, KCal::ResourceGroupwise *resource )
{
  ReadCalendarJob *job = new ReadCalendarJob( mSoap, mUrl, mSession, 0 );
  job->setCalendar( calendar );
  job->setCalendarFolder( &mCalendarFolder );
  job->setResource( resource );

  connect( job, SIGNAL( done() ),
           this, SIGNAL( readCalendarFinished() ) );

  mWeaver->enqueue( job );

  return true;
}

bool GroupwiseServer::readFreeBusy( const QString &email, 
  const QDate &start, const QDate &end, KCal::FreeBusy *freeBusy )
{
  kdDebug() << "GroupwiseServer::readFreeBusy()" << endl;

  GWConverter conv( mSoap );

  // Setup input data
  ns1__FreeBusyUser user;
  user.email = email.utf8();

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

  mSoap->header->ns1__session = mSession;

  int result = soap_call___ns13__startFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &startSessionRequest, &startSessionResponse );

  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  }

  int fbSessionId = startSessionResponse.freeBusySessionId;

  kdDebug() << "Free/Busy session ID: " << fbSessionId << endl;


  // Get free/busy data
  _ns1__getFreeBusyRequest getFreeBusyRequest;
  getFreeBusyRequest.freeBusySessionId = QString::number( fbSessionId ).utf8();
  
  _ns1__getFreeBusyResponse getFreeBusyResponse;

  mSoap->header->ns1__session = mSession;

  result = soap_call___ns15__getFreeBusyRequest( mSoap,
    mUrl.latin1(), NULL, &getFreeBusyRequest, &getFreeBusyResponse );

  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
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
          kdDebug() << "BLOCK Subject: " << subject.c_str() << endl;

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

  mSoap->header->ns1__session = mSession;

  result = soap_call___ns14__closeFreeBusySessionRequest( mSoap,
    mUrl.latin1(), NULL, &closeSessionRequest, &closeSessionResponse );

  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return false;
  }


  return true;
}

#include "groupwiseserver.moc"
