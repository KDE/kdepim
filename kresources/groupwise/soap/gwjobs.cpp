/*
    This file is part of KDE.

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

#include <kdebug.h>
#include <klocale.h>
#include <kabc/addressee.h>
#include <kabc/resource.h>

#include <libkcal/incidence.h>
#include <libkcal/resourcecached.h>

#include <qtimer.h>

#include "contactconverter.h"
#include "incidenceconverter.h"
#include "soapH.h"
#include "groupwiseserver.h"

#include "gwjobs.h"

#define READ_ADDRESS_FOLDER_CHUNK_SIZE 250
#define READ_CALENDAR_FOLDER_CHUNK_SIZE 50

GWJob::GWJob( GroupwiseServer *server, struct soap *soap, const QString &url,
  const std::string &session )
  : mServer( server ), mSoap( soap ), mUrl( url ), mSession( session )
{
}

ReadAddressBooksJob::ReadAddressBooksJob( GroupwiseServer *server,
  struct soap *soap, const QString &url, const std::string &session )
  : GWJob( server, soap, url, session )
{
}

void ReadAddressBooksJob::setAddressBookIds( const QStringList &ids )
{
  mAddressBookIds = ids;

  kDebug() << "ADDR IDS: " << ids.join( "," ) << endl;
}

void ReadAddressBooksJob::run()
{
  kDebug() << "ReadAddressBooksJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getAddressBookListRequest addressBookListRequest;
  _ngwm__getAddressBookListResponse addressBookListResponse;
  soap_call___ngw__getAddressBookListRequest( mSoap, mUrl.toLatin1(),
                                              NULL, &addressBookListRequest, &addressBookListResponse );
  soap_print_fault( mSoap, stderr );

  if ( addressBookListResponse.books ) {
    std::vector<class ngwt__AddressBook * > *addressBooks = &addressBookListResponse.books->book;

    mServer->emitReadAddressBookTotalSize( ( mAddressBookIds.count() )
      * 100 );
    mProgress = 0;

    std::vector<class ngwt__AddressBook * >::const_iterator it;
    for ( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      if ( !(*it)->id ) {
        kError() << "No addressbook id" << endl;
      } else {
        QString id = GWConverter::stringToQString( (*it)->id );
        kDebug() << "Reading ID: " << id << endl;
        if ( mAddressBookIds.find( id ) != mAddressBookIds.end() ) {
          readAddressBook( *(*it)->id );
          mProgress += 100;
        }
      }
    }
  }
}

void ReadAddressBooksJob::readAddressBook( std::string &id )
{
  kDebug() << "ReadAddressBookJob::readAddressBook() " << id.c_str() << endl;
#if 0
  _ngwm__getItemsRequest itemsRequest;
  itemsRequest.container = &id;
  itemsRequest.count = -1;
  itemsRequest.filter = 0;
  itemsRequest.items = 0;
  itemsRequest.view = 0;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getItemsResponse itemsResponse;
  int result = soap_call___ngw__getItemsRequest( mSoap, mUrl.toLatin1(), 0,
                                    &itemsRequest, &itemsResponse );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    mServer->emitErrorMessage( i18n("Unable to read GroupWise address book: %1" ).arg( id.c_str() ), false );
    return;
  }

  std::vector<class ngwt__Item * > *items = &itemsResponse.items->item;
  if ( items ) {
#if 1
    kDebug() << "ReadAddressBooksJob::readAddressBook() - got " << items->size() << " contacts in folder " << id.c_str()   << endl;
#endif
    ContactConverter converter( mSoap );

    int maxCount = items->size();
    int count = 0;

    std::vector<class ngwt__Item * >::const_iterator it;
    for ( it = items->begin(); it != items->end(); ++it ) {
      ngwt__Item *item = *it;

#if 1
    if ( item )
      if ( item->name )
        kDebug() << "ITEM: " << item->name->c_str() << endl;
      if ( item->id )
        kDebug() << "ITEM: (" << item->id->c_str()
        << ")" << endl;
    else 
      kDebug() << "ITEM is null" << endl;
#endif

      ngwt__Contact *contact = dynamic_cast<ngwt__Contact *>( item );

      KABC::Addressee addr = converter.convertFromContact( contact );
      if ( !addr.isEmpty() ) {
        addr.setResource( mResource );
      
        addr.insertCustom( "GWRESOURCE", "CONTAINER", converter.stringToQString( id ) );

        QString remoteUid = converter.stringToQString( (*it)->id );

        KABC::Addressee oldAddressee = mResource->findByUid( mResource->idMapper().localId( remoteUid ) );
        if ( oldAddressee.isEmpty() ) // new addressee
          mResource->idMapper().setRemoteId( addr.uid(), remoteUid );
        else {
          addr.setUid( oldAddressee.uid() );
          mResource->removeAddressee( oldAddressee );
        }

        mResource->insertAddressee( addr );
        mResource->clearChange( addr );
      }

      int progress = int( mProgress + count++ * 100. / maxCount );

      kDebug() << "PROGRESS: mProgress: " << mProgress << " count: "
        << count << " maxCount: " << maxCount << " progress: " << progress
        << endl;

      mServer->emitReadAddressBookProcessedSize( progress );
    }
  }
#else
  unsigned int readItems = 0;
  unsigned int readChunkSize = READ_ADDRESS_FOLDER_CHUNK_SIZE;
  
  int cursor;

  _ngwm__createCursorRequest cursorRequest;
  _ngwm__createCursorResponse cursorResponse;

  cursorRequest.container = id;
  cursorRequest.view = 0;
  // filter for Contacts until we support Groups
  cursorRequest.filter = soap_new_ngwt__Filter( mSoap, -1 );
  ngwt__FilterEntry * fe = soap_new_ngwt__FilterEntry( mSoap, -1 );
  fe->op = isOf;
  fe->field = soap_new_std__string( mSoap, -1 );
  fe->field->append( "@type" );
  fe->value = soap_new_std__string( mSoap, -1 );
  fe->value->append( "Contact" );
  fe->custom = 0;
  fe->date = 0;

  cursorRequest.filter->element = fe;

  mSoap->header->ngwt__session = mSession;
  soap_call___ngw__createCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                        &cursorRequest,
                                        &cursorResponse );
  if ( cursorResponse.cursor )
    cursor = *(cursorResponse.cursor);
  else /* signal error? */
    return;

  _ngwm__readCursorRequest readCursorRequest;

  readCursorRequest.cursor = cursor;
  readCursorRequest.container = id;
  readCursorRequest.forward = true;
  readCursorRequest.position = 0;

  readCursorRequest.count = (int*)soap_malloc( mSoap, sizeof(int) );
  *( readCursorRequest.count ) = (int)readChunkSize;

  while ( true )
  {
    KABC::Addressee::List contacts;
    mSoap->header->ngwt__session = mSession;
    kDebug() << "sending readCursorRequest with session: " << mSession.c_str() << endl;
    _ngwm__readCursorResponse readCursorResponse;
    if ( soap_call___ngw__readCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                      &readCursorRequest,
                                      &readCursorResponse ) != SOAP_OK )
    {
      kDebug() << "Faults according to GSOAP:" << endl;
      soap_print_fault(mSoap, stderr);
      kDebug() << "Unable to read " << *( readCursorRequest.count ) << " items at once, halving number and retrying request" << endl;
      *( readCursorRequest.count ) = qMax( 1, *( readCursorRequest.count )/2 );
      continue;
    }
  
    if ( readCursorResponse.items ) {
      ContactConverter converter( mSoap );
 
      kDebug() << "ReadAddressBooksJob::readAddressBook() - got " << readCursorResponse.items->item.size() << " contacts in cursor read of folder " << id.c_str()   << endl;

      std::vector<class ngwt__Item * >::const_iterator it;
      for( it = readCursorResponse.items->item.begin(); it != readCursorResponse.items->item.end(); ++it ) {
        ngwt__Item *item = *it;

#if 1
        if ( item )
          if ( item->name )
            kDebug() << "ITEM: " << item->name->c_str() << endl;
          if ( item->id )
            kDebug() << "ITEM: (" << item->id->c_str()
            << ")" << endl;
        else 
          kDebug() << "ITEM is null" << endl;
#endif
        ngwt__Contact *contact = dynamic_cast<ngwt__Contact *>( item );
        KABC::Addressee addr;
        if ( contact )
        {
          addr = converter.convertFromContact( contact );
        }
        else if ( ngwt__Resource *resource = dynamic_cast<ngwt__Resource *>( item ) )
        {
          addr = converter.convertFromResource( resource );
        }
        else if ( ngwt__Group * group = dynamic_cast<ngwt__Group *>( item ) )
        {
          addr = converter.convertFromGroup( group ); 
        }
        if ( !addr.isEmpty() )
          contacts.append( addr );
      }
      readItems += readCursorResponse.items->item.size(); // this means that the read count is increased even if the call fails, but at least the while will always end
      kDebug() << " just read " << readCursorResponse.items->item.size() << " items" << endl;
      if ( readCursorResponse.items->item.size() == 0 )
        break;
    }
    else
    {
      kDebug() << " readCursor got no Items in Response!" << endl;
      break;
    }
    // pass the received addressees back to the server
    mServer->emitGotAddressees( contacts );
  }

  _ngwm__destroyCursorRequest destReq;
  _ngwm__destroyCursorResponse destResp;
  destReq.container = id;
  destReq.cursor = cursor;
  mSoap->header->ngwt__session = mSession;
  if ( soap_call___ngw__destroyCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                    &destReq,
                                    &destResp ) != SOAP_OK )
  {
    kDebug() << "Faults according to GSOAP:" << endl;
    soap_print_fault(mSoap, stderr);
  }

  kDebug() << " read " << readItems << " items in total" << endl;
#endif
}

ReadCalendarJob::ReadCalendarJob( GroupwiseServer *server, struct soap *soap, const QString &url,
                                  const std::string &session )
  : GWJob( server, soap, url, session ), mCalendar( 0 )
{
  kDebug() << "ReadCalendarJob()" << endl;
}

void ReadCalendarJob::setCalendarFolder( std::string *calendarFolder )
{
  mCalendarFolder = calendarFolder;
}

void ReadCalendarJob::setChecklistFolder( std::string *checklistFolder )
{
  mChecklistFolder = checklistFolder;
}

void ReadCalendarJob::setCalendar( KCal::Calendar *calendar )
{
  mCalendar = calendar;
}

void ReadCalendarJob::run()
{
  kDebug() << "ReadCalendarJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getFolderListRequest folderListReq;
  folderListReq.parent = "folders";
  folderListReq.view = 0;
  folderListReq.recurse = true;
  _ngwm__getFolderListResponse folderListRes;
  soap_call___ngw__getFolderListRequest( mSoap, mUrl.toLatin1(), 0,
                                         &folderListReq,
                                         &folderListRes );

  // consistency check variables
  unsigned int totalItems = 0;
  ReadItemCounts totals;
  totals.tasks = 0;
  totals.notes = 0;
  totals.appointments = 0;

  if ( folderListRes.folders ) {
    std::vector<class ngwt__Folder * > *folders = &folderListRes.folders->folder;
    if ( folders ) {
      std::vector<class ngwt__Folder * >::const_iterator it;
      for ( it = folders->begin(); it != folders->end(); ++it ) {
        if ( !(*it)->id ) {
          kError() << "No calendar id" << endl;
        } else {
          ngwt__SystemFolder * fld = dynamic_cast<ngwt__SystemFolder *>( *it );
          if ( fld )
          {
            bool haveReadFolder = false;
            int count = 0;
            ReadItemCounts itemCounts;
            itemCounts.appointments = 0;
            itemCounts.notes = 0;
            itemCounts.tasks = 0;
            if ( fld->count )
            {
              count = *( fld->count );
              totalItems += count;
            }
            kDebug() << "Folder " <<  (*(*it)->id).c_str() << ", containing " << count << " items." << endl;
            if ( fld->folderType == Calendar ) {
              kDebug() << "Reading folder " <<  (*(*it)->id).c_str() << ", of type Calendar, physically containing " << count << " items." << endl;
              readCalendarFolder( *(*it)->id, itemCounts );
              haveReadFolder = true;
              *mCalendarFolder = *((*it)->id);
            }
            else if ( fld->folderType == Checklist ) {
              kDebug() << "Reading folder " <<  (*(*it)->id).c_str() << ", of type Checklist, physically containing " << count << " items." << endl;
              readCalendarFolder( *(*it)->id, itemCounts );
              haveReadFolder = true;
              *mChecklistFolder = *((*it)->id);
            }
/*            else if ( fld->folderType == SentItems ) {
              kDebug() << "Reading folder " <<  (*(*it)->id).c_str() << ", of type SentItems, physically containing " << count << " items." << endl;
              readCalendarFolder( *(*it)->id, itemCounts );
              haveReadFolder = true;
              *mChecklistFolder = *((*it)->id);
            }*/
/*            else if ( fld->folderType == Mailbox ) {
              kDebug() << "Reading folder " <<  (*(*it)->id).c_str() << ", of type Mailbox (not yet accepted items), containing " << count << " items." << endl;
              readCalendarFolder( *(*it)->id, count, itemCounts );
              haveReadFolder = true;
            }*/
            if ( haveReadFolder )
            {
              kDebug() << "Folder contained " << itemCounts.appointments << " appointments, " << itemCounts.notes << " notes, and " << itemCounts.tasks << " tasks." << endl;
              totals.appointments += itemCounts.appointments;
              totals.notes += itemCounts.notes;
              totals.tasks += itemCounts.tasks;
            }
            else
              kDebug() << "Skipping folder: " << *((*it)->id )->c_str() << endl;
          }
        }
      }
    }
  }

  // perform consistency checks
  kDebug() << "Total count of items of all types in folders we read: " << totalItems << endl;
  kDebug() << "Folders we read contained " << totals.appointments << " appointments, " << totals.notes << " notes, and " << totals.tasks << " tasks." << endl;
  kDebug() << "Local calendar now contains " << mCalendar->rawEvents().count() << " events and " << mCalendar->rawJournals().count() << " journals, and " << mCalendar->rawTodos().count() << " todos." << endl;
  if ( totals.appointments == mCalendar->rawEvents().count() )
    kDebug() << "All events accounted for." << endl;
  else
    kDebug() << "ERROR: event counts do not match." << endl;
  if ( totals.notes == mCalendar->rawJournals().count() )
    kDebug() << "All journals accounted for." << endl;
  else
    kDebug() << "ERROR: journal counts do not match." << endl;
  if ( totals.tasks == mCalendar->rawTodos().count() )
    kDebug() << "All todos accounted for." << endl;
  else
    kDebug() << "ERROR: todo counts do not match." << endl;

  kDebug() << "ReadCalendarJob::run() done" << endl;
}

void ReadCalendarJob::readCalendarFolder( const std::string &id, ReadItemCounts & counts )
{
  kDebug() << "ReadCalendarJob::readCalendarFolder() '" << id.c_str() << endl;
  mSoap->header->ngwt__session = mSession;

#if 0
  _ngwm__getItemsRequest itemsRequest;

  itemsRequest.container = id;
  std::string *str = soap_new_std__string( mSoap, -1 );
  str->append( "startDate endDate subject alarm allDayEvent place timezone iCalId recipients message recipientStatus recurrenceKey" );
  itemsRequest.view = str;
  itemsRequest.filter = 0;
  itemsRequest.items = 0;
  itemsRequest.count = -1;

/*
  ngwt__Filter *filter = soap_new_ngwm__Filter( mSoap, -1 );
  ngwt__FilterEntry *filterEntry = soap_new_ngwm__FilterEntry( mSoap, -1 );
  filterEntry->op = gte;
  filterEntry->field = QString::fromLatin1( "startDate" ).utf8();
  filterEntry->value = QDateTime::currentDateTime().toString( "yyyyMMddThhmmZ" ).utf8();

  filter->element = filterEntry;

  itemsRequest.filter = filter;
*/

  _ngwm__getItemsResponse itemsResponse;
  soap_call___ngw__getItemsRequest( mSoap, mUrl.toLatin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  kDebug() << "Faults according to GSOAP:" << endl;
  soap_print_fault(mSoap, stderr);

  if ( itemsResponse.items ) {
    IncidenceConverter conv( mSoap );

    std::vector<class ngwt__Item * >::const_iterator it;
    for( it = itemsResponse.items->item.begin(); it != itemsResponse.items->item.end(); ++it ) {
      ngwt__Appointment *a = dynamic_cast<ngwt__Appointment *>( *it );
      KCal::Incidence *i = 0;
      if ( a ) {
        i = conv.convertFromAppointment( a );
      } else {
        ngwt__Task *t = dynamic_cast<ngwt__Task *>( *it );
        if ( t ) {
          i = conv.convertFromTask( t );
        }
      }

      if ( i ) {
        i->setCustomProperty( "GWRESOURCE", "CONTAINER", conv.stringToQString( id ) );

        mCalendar->addIncidence( i );
      }
    }
  }
#else
  unsigned int readItems = 0;
  unsigned int readChunkSize = READ_CALENDAR_FOLDER_CHUNK_SIZE;
  
  int cursor;

  _ngwm__createCursorRequest cursorRequest;
  _ngwm__createCursorResponse cursorResponse;

  cursorRequest.container = id;
#if 1
  cursorRequest.view = soap_new_std__string( mSoap, -1 );
  cursorRequest.view->append( "default message recipients attachments recipientStatus peek" /*"container status source security distribution acceptLevel startDate endDate subject alarm allDayEvent place timezone iCalId recipients message recurrenceKey"*/ );

#else
  cursorRequest.view = 0;
#endif
  cursorRequest.filter = 0;

  soap_call___ngw__createCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                        &cursorRequest,
                                        &cursorResponse );
  if ( cursorResponse.cursor )
    cursor = *(cursorResponse.cursor);
  else /* signal error? */
    return;

  _ngwm__readCursorRequest readCursorRequest;

  readCursorRequest.cursor = cursor;
  readCursorRequest.container = id;
  readCursorRequest.forward = true;
#if 0 // seeing if adding the position enum causes the server to truncate returned data
  readCursorRequest.position = (ngwt__CursorSeek*)soap_malloc( mSoap, sizeof(ngwt__CursorSeek) );
  *( readCursorRequest.position ) = start;
#else
  readCursorRequest.position = 0;
#endif
  readCursorRequest.count = (int*)soap_malloc( mSoap, sizeof(int) );
  *( readCursorRequest.count ) = (int)readChunkSize;

  //soap_set_imode(mSoap, SOAP_XML_STRICT);

  while ( true )
  {
    mSoap->header->ngwt__session = mSession;
    kDebug() << "sending readCursorRequest with session: " << mSession.c_str() << endl;
    _ngwm__readCursorResponse readCursorResponse;
    if ( soap_call___ngw__readCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                      &readCursorRequest,
                                      &readCursorResponse ) != SOAP_OK )
    {
      kDebug() << "Faults according to GSOAP:" << endl;
      soap_print_fault(mSoap, stderr);
      soap_print_fault_location(mSoap, stderr);

      kDebug() << "EXITING" << endl;
      break;
    }
  
    if ( readCursorResponse.items ) {
      IncidenceConverter conv( mSoap );
      conv.setFrom( mServer->userName(), mServer->userEmail(), mServer->userUuid() );

      std::vector<class ngwt__Item * >::const_iterator it;
      for( it = readCursorResponse.items->item.begin(); it != readCursorResponse.items->item.end(); ++it ) {
        KCal::Incidence *i = 0;
        ngwt__Appointment *a = dynamic_cast<ngwt__Appointment *>( *it );
        if ( a ) {
          i = conv.convertFromAppointment( a );
          counts.appointments++;
        } else {
          ngwt__Task *t = dynamic_cast<ngwt__Task *>( *it );
          if ( t ) {
            i = conv.convertFromTask( t );
            counts.tasks++;
          }
          else {
            ngwt__Note *n = dynamic_cast<ngwt__Note *>( *it );
            if ( n ) {
              i = conv.convertFromNote( n );
              counts.notes++;
            }
          }
        }
        if ( i ) {
          i->setCustomProperty( "GWRESOURCE", "CONTAINER", conv.stringToQString( id ) );
  
          mCalendar->addIncidence( i );
        }
      }
      readItems += readCursorResponse.items->item.size();
      kDebug() << " just read " << readCursorResponse.items->item.size() << " items" << endl;
      if ( readCursorResponse.items->item.size() == 0 )
        break;
    }
    else
    {
      kDebug() << " readCursor got no Items in Response!" << endl;
      mServer->emitErrorMessage( i18n("Unable to read GroupWise address book: reading %1 returned no items." ).arg( id.c_str() ), false );
      break;
    }
  }

  _ngwm__destroyCursorRequest destReq;
  _ngwm__destroyCursorResponse destResp;
  destReq.container = id;
  destReq.cursor = cursor;
  mSoap->header->ngwt__session = mSession;
  if ( soap_call___ngw__destroyCursorRequest( mSoap, mUrl.toLatin1(), 0,
                                    &destReq,
                                    &destResp ) != SOAP_OK )
  {
    kDebug() << "Faults according to GSOAP:" << endl;
    soap_print_fault(mSoap, stderr);
  }
 
  kDebug() << " read " << readItems << " items in total" << endl;
#endif
}

UpdateAddressBooksJob::UpdateAddressBooksJob( GroupwiseServer *server,
  struct soap *soap, const QString &url, const std::string &session )
  : GWJob( server, soap, url, session )
{
}

void UpdateAddressBooksJob::setAddressBookIds( const QStringList &ids )
{
  mAddressBookIds = ids;

  kDebug() << "ADDR IDS: " << ids.join( "," ) << endl;
}

void UpdateAddressBooksJob::setStartSequenceNumber( const int startSeqNo )
{
  mStartSequenceNumber = startSeqNo;
}

void UpdateAddressBooksJob::run()
{
  kDebug() << "UpdateAddressBooksJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getDeltasRequest request;
  _ngwm__getDeltasResponse response;

  GWConverter conv( mSoap );
  request.container.append( mAddressBookIds.first().toLatin1() );
  request.deltaInfo = soap_new_ngwt__DeltaInfo( mSoap, -1 );
  request.deltaInfo->count = (int*)soap_malloc( mSoap, sizeof(int) );
  *( request.deltaInfo->count ) = -1;
 /* request.deltaInfo->count = 0;*/
  request.deltaInfo->lastTimePORebuild = 0;
  request.deltaInfo->firstSequence = (unsigned long*)soap_malloc( mSoap, sizeof(unsigned long) );
  *(request.deltaInfo->firstSequence) = mStartSequenceNumber;
  request.deltaInfo->lastSequence = 0; /*(unsigned long*)soap_malloc( mSoap, sizeof(unsigned long) );*/
  /* *(request.deltaInfo->lastSequence) = mLastSequenceNumber; */
  //request.view = soap_new_std__string( mSoap, -1 );
  //request.view->append("id name version modified ItemChanges");
  request.view = 0;
  int result = soap_call___ngw__getDeltasRequest( mSoap, mUrl.toLatin1(),
                                              NULL, &request, &response);
  soap_print_fault( mSoap, stderr );

  if (!mServer->checkResponse( result, response.status ) )
  {
    kdError() << "Error when getting addressbook deltas" << endl;
    return;
  }

  std::vector<class ngwt__Item * > *items = &response.items->item;
  if ( items ) {
#if 1
    kDebug() << "ReadAddressBooksJob::UpdateAddressBooksJob() - got " << items->size() << "contacts" << endl;
#endif
    KABC::Addressee::List contacts;
    ContactConverter converter( mSoap );

    std::vector<class ngwt__Item * >::const_iterator it;
    for ( it = items->begin(); it != items->end(); ++it ) {
      ngwt__Item *item = *it;

#if 1
    if ( item )
      if ( item->name )
        kDebug() << "ITEM: " << item->name->c_str() << endl;
      if ( item->id )
        kDebug() << "ITEM: ID (" << item->id->c_str()
        << ")" << endl;
    else 
      kDebug() << "ITEM is null" << endl;
#endif

      ngwt__Contact *contact = dynamic_cast<ngwt__Contact *>( item );

      KABC::Addressee addr = converter.convertFromContact( contact );
      if ( !addr.isEmpty() )
        contacts.append( addr );
    }
    mServer->emitGotAddressees( contacts );
  }

//   if ( addressBookListResponse.books ) { 
//     std::vector<class ngwt__AddressBook * > *addressBooks = &addressBookListResponse.books->book;
}
