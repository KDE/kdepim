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
#include <libkcal/calendar.h>
#include <libkcal/incidence.h>

#include <qtimer.h>

#include "contactconverter.h"
#include "incidenceconverter.h"
#include "kabc_resourcegroupwise.h"
#include "kcal_resourcegroupwise.h"
#include "soapH.h"

#include "gwjobs.h"

GWJob::GWJob( Type type, struct soap *soap, const QString &url, const std::string &session,
              QObject *parent )
  : Job( parent, "GWJob" ),
    mType( type ), mUrl( url ), mSession( session )
{
  mSoap = soap_copy( soap );
  mSoap->header = new( SOAP_ENV__Header );
}

GWJob::~GWJob()
{
  soap_free( mSoap );
  mSoap = 0;
}

void GWJob::processEvent( KPIM::ThreadWeaver::Event *event )
{
  KPIM::ThreadWeaver::Job::processEvent( event );
  if ( event->action() == KPIM::ThreadWeaver::Event::JobFinished )
    deleteLater();
}

ReadAddressBooksJob::ReadAddressBooksJob( struct soap *soap, const QString &url,
                                          const std::string &session, QObject *parent )
  : GWJob( ReadAddressBooks, soap, url, session, parent )
{
}

void ReadAddressBooksJob::setAddressBookIds( const QStringList &ids )
{
  mAddressBookIds = ids;
}

void ReadAddressBooksJob::setResource( KABC::ResourceGroupwise *resource )
{
  mResource = resource;
}

void ReadAddressBooksJob::run()
{
  mSoap->header->ns1__session = mSession;
  _ns1__getAddressBookListResponse addressBookListResponse;
  soap_call___ns4__getAddressBookListRequest( mSoap, mUrl.latin1(),
                                              NULL, "", &addressBookListResponse );
  soap_print_fault( mSoap, stderr );

  if ( addressBookListResponse.books ) {
    std::vector<class ns1__AddressBook * > *addressBooks = addressBookListResponse.books->book;
    std::vector<class ns1__AddressBook * >::const_iterator it;
    for ( it = addressBooks->begin(); it != addressBooks->end(); ++it ) {
      if ( mAddressBookIds.find( GWConverter::stringToQString( (*it)->id ) ) != mAddressBookIds.end() )
        readAddressBook( (*it)->id );
    }
  }
}

void ReadAddressBooksJob::readAddressBook( std::string &id )
{
  _ns1__getItemsRequest itemsRequest;
  itemsRequest.container = id;
  itemsRequest.filter = 0;
  itemsRequest.items = 0;

  mSoap->header->ns1__session = mSession;
  _ns1__getItemsResponse itemsResponse;
  soap_call___ns6__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest, &itemsResponse );

  std::vector<class ns1__Item * > *items = itemsResponse.items->item;
  if ( items ) {
    ContactConverter converter( mSoap );

    std::vector<class ns1__Item * >::const_iterator it;
    for ( it = items->begin(); it != items->end(); ++it ) {
      _ns1__getItemRequest itemRequest;
      itemRequest.id = (*it)->id;
      itemRequest.view = 0;

      mSoap->header->ns1__session = mSession;
      _ns1__getItemResponse itemResponse;
      soap_call___ns5__getItemRequest( mSoap, mUrl.latin1(), 0,
                                       &itemRequest, &itemResponse );

      ns1__Item *item = itemResponse.item;
      ns1__Contact *contact = dynamic_cast<ns1__Contact *>( item );

      KABC::Addressee addr = converter.convertFromContact( contact );
      if ( !addr.isEmpty() ) {
        addr.insertCustom( "GWRESOURCE", "CONTAINER", converter.stringToQString( id ) );

        QString remoteUid = converter.stringToQString( (*it)->id );

        KABC::Addressee oldAddressee = mResource->findByUid( mResource->localUid( remoteUid ) );
        if ( oldAddressee.isEmpty() ) // new addressee
          mResource->setRemoteUid( addr.uid(), remoteUid );
        else {
          addr.setUid( oldAddressee.uid() );
          mResource->removeAddressee( oldAddressee );
        }

        mResource->insertAddressee( addr );
        mResource->clearChange( addr );
      }
    }
  }
}

ReadCalendarJob::ReadCalendarJob( struct soap *soap, const QString &url,
                                  const std::string &session, QObject *parent )
  : GWJob( ReadCalendar, soap, url, session, parent )
{
}

void ReadCalendarJob::setCalendar( KCal::Calendar *calendar )
{
  mCalendar = calendar;
}

void ReadCalendarJob::setCalendarFolder( std::string *calendarFolder )
{
  mCalendarFolder = calendarFolder;
}

void ReadCalendarJob::setResource( KCal::ResourceGroupwise *resource )
{
  mResource = resource;
}

void ReadCalendarJob::run()
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
        if ( (*it)->type && *((*it)->type) == "Calendar" ) {
          readCalendarFolder( (*it)->id );
          (*mCalendarFolder) = (*it)->id;
        }
      }
    }
  }
}

void ReadCalendarJob::readCalendarFolder( const std::string &id )
{
  _ns1__getItemsRequest itemsRequest;

  itemsRequest.container = id;
  itemsRequest.view = "recipients message recipientStatus";

/*
  ns1__Filter *filter = soap_new_ns1__Filter( mSoap, -1 );
  ns1__FilterEntry *filterEntry = soap_new_ns1__FilterEntry( mSoap, -1 );
  filterEntry->op = gte;
  filterEntry->field = QString::fromLatin1( "startDate" ).utf8();
  filterEntry->value = QDateTime::currentDateTime().toString( "yyyyMMddThhmmZ" ).utf8();

  filter->element = filterEntry;

  itemsRequest.filter = filter;
*/
  itemsRequest.filter = 0;
  itemsRequest.items = 0;

  mSoap->header->ns1__session = mSession;
  _ns1__getItemsResponse itemsResponse;
  soap_call___ns6__getItemsRequest( mSoap, mUrl.latin1(), 0,
                                    &itemsRequest,
                                    &itemsResponse );
  soap_print_fault(mSoap, stderr);

  std::vector<class ns1__Item * > *items = itemsResponse.items->item;

  if ( items ) {
    IncidenceConverter conv( mSoap );

    std::vector<class ns1__Item * >::const_iterator it;
    for( it = items->begin(); it != items->end(); ++it ) {
      ns1__Appointment *a = dynamic_cast<ns1__Appointment *>( *it );
      KCal::Incidence *i = 0;
      if ( a ) {
        i = conv.convertFromAppointment( a );
      } else {
        ns1__Task *t = dynamic_cast<ns1__Task *>( *it );
        if ( t ) {
          i = conv.convertFromTask( t );
        }
      }

      if ( i ) {
        i->setCustomProperty( "GWRESOURCE", "CONTAINER", conv.stringToQString( id ) );

        QString remoteUid = conv.stringToQString( (*it)->id );
        KCal::Incidence *oldIncidence = mResource->event( mResource->localUid( remoteUid ) );
        if ( !oldIncidence )
          oldIncidence = mResource->todo( mResource->localUid( remoteUid ) );

        if ( !oldIncidence ) { // new incidence
          mResource->setRemoteUid( i->uid(), remoteUid );
        } else {
          i->setUid( oldIncidence->uid() );
          mCalendar->deleteIncidence( oldIncidence );
        }

        mCalendar->addIncidence( i );
      }
    }
  }
}
