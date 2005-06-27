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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kabc/addressee.h>
#include <kdebug.h>
#include <libkcal/incidence.h>
#include <libkcal/resourcecached.h>
#include <kabcresourcecached.h>

#include <qtimer.h>

#include "contactconverter.h"
#include "incidenceconverter.h"
#include "soapH.h"
#include "groupwiseserver.h"

#include "gwjobs.h"

#define READ_FOLDER_CHUNK_SIZE 20

GWJob::GWJob( struct soap *soap, const QString &url,
  const std::string &session )
  : mSoap( soap ), mUrl( url ), mSession( session )
{
}

ReadAddressBooksJob::ReadAddressBooksJob( GroupwiseServer *server,
  struct soap *soap, const QString &url, const std::string &session )
  : GWJob( soap, url, session ), mServer( server )
{
}

void ReadAddressBooksJob::setAddressBookIds( const QStringList &ids )
{
  mAddressBookIds = ids;

  kdDebug() << "ADDR IDS: " << ids.join( "," ) << endl;
}

void ReadAddressBooksJob::setResource( KABC::ResourceCached *resource )
{
  mResource = resource;
}

void ReadAddressBooksJob::run()
{
  kdDebug() << "ReadAddressBooksJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getAddressBookListRequest addressBookListRequest;
  _ngwm__getAddressBookListResponse addressBookListResponse;
  soap_call___ngw__getAddressBookListRequest( mSoap, mUrl.latin1(),
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
        kdError() << "No addressbook id" << endl;
      } else {
        QString id = GWConverter::stringToQString( (*it)->id );
        kdDebug() << "ID: " << id << endl;
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
  kdDebug() << "ReadAddressBookJob::readAddressBook() " << id.c_str() << endl;

  _ngwm__getItemsRequest itemsRequest;
  itemsRequest.container = &id;
  itemsRequest.count = -1;
  itemsRequest.filter = 0;
  itemsRequest.items = 0;
  itemsRequest.view = 0;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getItemsResponse itemsResponse;
  int result = soap_call___ngw__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest, &itemsResponse );
  if ( result != 0 ) {
    soap_print_fault( mSoap, stderr );
    return;
  }

  std::vector<class ngwt__Item * > *items = &itemsResponse.items->item;
  if ( items ) {
#if 1
    kdDebug() << "ReadAddressBooksJob::readAddressBook() - got " << items->size() << "contacts" << endl;
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
        kdDebug() << "ITEM: " << item->name->c_str() << endl;
      if ( item->id )
        kdDebug() << "ITEM: (" << item->id->c_str()
        << ")" << endl;
    else 
      kdDebug() << "ITEM is null" << endl;
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

      kdDebug() << "PROGRESS: mProgress: " << mProgress << " count: "
        << count << " maxCount: " << maxCount << " progress: " << progress
        << endl;

      mServer->emitReadAddressBookProcessedSize( progress );
    }
  }
}

ReadCalendarJob::ReadCalendarJob( struct soap *soap, const QString &url,
                                  const std::string &session )
  : GWJob( soap, url, session ), mCalendar( 0 )
{
  kdDebug() << "ReadCalendarJob()" << endl;
}

void ReadCalendarJob::setCalendarFolder( std::string *calendarFolder )
{
  mCalendarFolder = calendarFolder;
}

void ReadCalendarJob::setCalendar( KCal::Calendar *calendar )
{
  mCalendar = calendar;
}

void ReadCalendarJob::run()
{
  kdDebug() << "ReadCalendarJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getFolderListRequest folderListReq;
  folderListReq.parent = "folders";
  folderListReq.view = 0;
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
        if ( !(*it)->id ) {
          kdError() << "No calendar id" << endl;
        } else {
          ngwt__SystemFolder * fld = dynamic_cast<ngwt__SystemFolder *>( *it );
          if ( fld )
            if ( fld->folderType == Calendar || fld->folderType == Checklist ) {
              kdDebug() << "Got calendar folder" << endl;
              int count = 0;
              if ( fld->count )
                count = *( fld->count );
              readCalendarFolder( *(*it)->id, count );
              (*mCalendarFolder) = *(*it)->id;
          }
        }
      }
    }
  }
  
  kdDebug() << "ReadCalendarJob::run() done" << endl;
}

void ReadCalendarJob::readCalendarFolder( const std::string &id, const unsigned int count )
{
  kdDebug() << "ReadCalendarJob::readCalendarFolder() '" << id.c_str() << "', with " << count << " entries." << endl;
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
  soap_call___ngw__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  kdDebug() << "Faults according to GSOAP:" << endl;
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
  unsigned int readChunkSize = QMIN( READ_FOLDER_CHUNK_SIZE, count);
  
  int cursor;

  _ngwm__createCursorRequest cursorRequest;
  _ngwm__createCursorResponse cursorResponse;

  cursorRequest.container = id;
#if 0
  cursorRequest.view = soap_new_std__string( mSoap, -1 );
  cursorRequest.view->append( "startDate endDate subject alarm allDayEvent place timezone iCalId recipients message recipientStatus recurrenceKey" );
#else
  cursorRequest.view = 0;
#endif
  cursorRequest.filter = 0;

  soap_call___ngw__createCursorRequest( mSoap, mUrl.latin1(), 0,
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

  while ( readItems < count )
  {
    mSoap->header->ngwt__session = mSession;
    kdDebug() << "sending readCursorRequest with session: " << mSession.c_str() << endl;
    _ngwm__readCursorResponse readCursorResponse;
    if ( soap_call___ngw__readCursorRequest( mSoap, mUrl.latin1(), 0,
                                      &readCursorRequest,
                                      &readCursorResponse ) != SOAP_OK )
    {
      kdDebug() << "Faults according to GSOAP:" << endl;
      soap_print_fault(mSoap, stderr);
      kdDebug() << "EXITING" << endl;
      return;
    }
  
    if ( readCursorResponse.items ) {
      IncidenceConverter conv( mSoap );
  
      std::vector<class ngwt__Item * >::const_iterator it;
      for( it = readCursorResponse.items->item.begin(); it != readCursorResponse.items->item.end(); ++it ) {
        KCal::Incidence *i = 0;
        ngwt__Appointment *a = dynamic_cast<ngwt__Appointment *>( *it );
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
    else
      kdDebug() << " readCursor got no Items in Response!" << endl;

    readItems += readChunkSize; // this means that the read count is increased even if the call fails, but at least the while will always end
    kdDebug() << " just read " << readChunkSize << " items" << endl;
    readChunkSize = QMIN( readChunkSize, count - readItems );
    *(readCursorRequest.count) = readChunkSize;
    //*(readCursorRequest.position) = current;
  }
  kdDebug() << " read " << readItems << " items in total" << endl;
  
#endif
}

UpdateAddressBooksJob::UpdateAddressBooksJob( GroupwiseServer *server,
  struct soap *soap, const QString &url, const std::string &session )
  : GWJob( soap, url, session ), mServer( server )
{
}

void UpdateAddressBooksJob::setAddressBookIds( const QStringList &ids )
{
  mAddressBookIds = ids;

  kdDebug() << "ADDR IDS: " << ids.join( "," ) << endl;
}

void UpdateAddressBooksJob::setResource( KABC::ResourceCached *resource )
{
  mResource = resource;
}

void UpdateAddressBooksJob::setStartSequenceNumber( const int startSeqNo )
{
  mStartSequenceNumber = startSeqNo;
}

void UpdateAddressBooksJob::run()
{
  kdDebug() << "UpdateAddressBooksJob::run()" << endl;

  mSoap->header->ngwt__session = mSession;
  _ngwm__getDeltasRequest request;
  _ngwm__getDeltasResponse response;

  GWConverter conv( mSoap );
  request.container.append( mAddressBookIds.first().latin1() );
  request.deltaInfo = soap_new_ngwt__DeltaInfo( mSoap, -1 );
  request.deltaInfo->count = (int*)soap_malloc( mSoap, sizeof(int) );
  *( request.deltaInfo->count ) = -1;
 /* request.deltaInfo->count = 0;*/
  request.deltaInfo->lastTimePORebuild = 0;
  request.deltaInfo->firstSequence = (unsigned long*)soap_malloc( mSoap, sizeof(unsigned long) );
  *(request.deltaInfo->firstSequence) = mStartSequenceNumber;
  request.deltaInfo->lastSequence = 0; /*(unsigned long*)soap_malloc( mSoap, sizeof(unsigned long) );*/
  /* *(request.deltaInfo->lastSequence) = mLastSequenceNumber; */
  request.view = soap_new_std__string( mSoap, -1 );
  //request.view->append("id name version modified ItemChanges");
  soap_call___ngw__getDeltasRequest( mSoap, mUrl.latin1(),
                                              NULL, &request, &response);
  soap_print_fault( mSoap, stderr );

//   if ( addressBookListResponse.books ) { 
//     std::vector<class ngwt__AddressBook * > *addressBooks = &addressBookListResponse.books->book;
}
