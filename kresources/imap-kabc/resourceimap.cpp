/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klarälvdalens Datakonsult AB 
        <info@klaralvdalens-datakonsult.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <dcopclient.h>

#include <kabc/formatfactory.h>
#include <kapp.h>
#include <kdcopservicestarter.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>

#include <qstring.h>

#include "kmailicalIface_stub.h"

#include "resourceimap.h"
#include <kmessagebox.h>


static const QCString dcopObjectId = "KMailICalIface";

class IMAPFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KABC::ResourceIMAP( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

extern "C"
{
  void *init_kabc_imap()
  {
    return ( new IMAPFactory() );
  }
}


KABC::ResourceIMAP::ResourceIMAP( const KConfig *config )
  : DCOPObject("ResourceIMAP-KABC"), KABC::Resource( config ), mSilent( false )
{
  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( "vcard" );

  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();
  mDCOPClient->registerAs( "resourceimap-kabc", true );

  // Make the connection to KMail ready
  mKMailIcalIfaceStub = 0;
  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
           this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
}

KABC::ResourceIMAP::~ResourceIMAP()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  delete mDCOPClient;
  delete mFormat;
}

bool KABC::ResourceIMAP::doOpen()
{
  return connectToKMail();
}

KABC::Ticket * KABC::ResourceIMAP::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdError() << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void KABC::ResourceIMAP::releaseSaveTicket( Ticket* ticket )
{
  delete ticket;
}

bool KABC::ResourceIMAP::load()
{
  mAddrMap.clear();

  if ( !connectToKMail() ) {
    kdError() << "DCOP error during incidences(QString)\n";
    return false;
  }

  QStringList lst = mKMailIcalIfaceStub->incidences( "Contact" );
  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::load()\n";
    return false;
  }

  for( QStringList::iterator it = lst.begin(); it != lst.end(); ++it ) {
    KABC::Addressee addr = mConverter.parseVCard( *it );
    addr.setResource( this );
    addr.setChanged( false );
    Resource::insertAddressee( addr );
  }

  return true;
}

bool KABC::ResourceIMAP::save( Ticket* )
{
  // Save all vCards in a QStringList
  QStringList vCards;
  for( ConstIterator  it = begin(); it != end(); ++it )
    if( (*it).changed() ) {
      // First the uid and then the vCard
      vCards << (*it).uid();
      vCards << mConverter.createVCard( *it );
    }

  if( vCards.isEmpty() )
    // Nothing to save, so return happy
    return true;

  // Save in KMail
  if( !connectToKMail() ) {
    kdError() << "DCOP error during ResourceIMAP::save()\n";
    return false;
  }
  mKMailIcalIfaceStub->update( "Contact", vCards );

  // Mark all of them as read
  for( Iterator it = begin(); it != end(); ++it )
    (*it).setChanged( false );

  return mKMailIcalIfaceStub->ok();
}

void KABC::ResourceIMAP::insertAddressee( const Addressee& addr )
{
  // Call kmail ...
  if ( !mSilent ) {
    // Check if we need to update or save this
    bool update = false;
    if( mAddrMap.contains( addr.uid() ) ) {
      // This address is already in the map
      if( !addr.changed() )
        // Not changed, no need to save it
        return;
      else
        update = true;
    }

    if ( !connectToKMail() ) {
      kdError() << "DCOP error during "
                << "ResourceIMAP::insertAddressee(const Addressee& addr)\n";
    } else {
      QString vCard = mConverter.createVCard( addr );

      if( !update )
        // Save the new addressee
        mKMailIcalIfaceStub->addIncidence( "Contact", addr.uid(), vCard );
      else
        // Update existing addressee
        mKMailIcalIfaceStub->update( "Contact", addr.uid(), vCard );

      if( mKMailIcalIfaceStub->ok() )
        // This is ugly, but it's faster than doing
        // mAddrMap.find(addr.uid()), which would give the same :-(
        // Reason for this: The Changed attribute of Addressee should
        // be mutable
        const_cast<Addressee&>(addr).setChanged( false );
    }
  }

  Resource::insertAddressee( addr );
}

void KABC::ResourceIMAP::removeAddressee( const Addressee& addr )
{
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during "
                << "ResourceIMAP::removeAddressee(const Addressee& addr)\n";
    } else {
      mKMailIcalIfaceStub->deleteIncidence( "Contact", addr.uid() );
    }
  }

  Resource::removeAddressee( addr );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KABC::ResourceIMAP::addIncidence( const QString& type,
				       const QString& vCard )
{
  if( type == "Contact" ) {
    bool silent = mSilent;
    mSilent = true;

    KABC::Addressee addr = mConverter.parseVCard( vCard );
    addr.setResource( this );
    addr.setChanged( false );
    mAddrMap.insert( addr.uid(), addr );

    addressBook()->emitAddressBookChanged();

    mSilent = silent;

    return true;
  }

  return false;
}

void KABC::ResourceIMAP::deleteIncidence( const QString& type,
					  const QString& uid )
{
  if( type == "Contact" ) {
    bool silent = mSilent;
    mSilent = true;

    mAddrMap.remove( uid );
    addressBook()->emitAddressBookChanged();

    mSilent = silent;
  }
}

void KABC::ResourceIMAP::slotRefresh( const QString& type )
{
  if( type == "Contact" ) {
    bool silent = mSilent;
    mSilent = true;

    load();
    addressBook()->emitAddressBookChanged();

    mSilent = silent;
  }
}

/*
 * KMail connection code
 */
bool KABC::ResourceIMAP::connectToKMail() const
{
  if ( !mKMailIcalIfaceStub ) {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->
      findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null,
                      QString::null, &error, &dcopService );
    if ( result != 0 ) {
      kdError() << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here, using e.g. KMessageBox
      return false;
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString)",
                              "addIncidence(QString,QString)" ) ) {
      kdError() << "DCOP connection to incidenceAdded failed" << endl;
    }
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString)",
                              "deleteIncidence(QString,QString)" ) ) {
      kdError() << "DCOP connection to incidenceDeleted failed" << endl;
    }
    if ( !connectKMailSignal( "signalRefresh(QString)",
                              "slotRefresh(QString)" ) ) {
      kdError() << "DCOP connection to signalRefresh failed" << endl;
    }
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool KABC::ResourceIMAP::connectKMailSignal( const QCString& signal,
					     const QCString& method ) const
{
  ResourceIMAP* _this = const_cast<ResourceIMAP*>( this );
  return _this->connectDCOPSignal( "kmail", dcopObjectId, signal, method,
                                   false );
}

void KABC::ResourceIMAP::unregisteredFromDCOP( const QCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0;
  }
}


#include "resourceimap.moc"
