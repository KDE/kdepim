/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 Klarälvdalens Datakonsult AB 
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
using namespace KABC;

static const QCString dcopObjectId = "KMailICalIface";

class IMAPFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new ResourceIMAP( config );
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


ResourceIMAP::ResourceIMAP( const KConfig *config )
  : DCOPObject("ResourceIMAP-KABC"), Resource( config ), mSilent( false )
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

ResourceIMAP::~ResourceIMAP()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  delete mDCOPClient;
  delete mFormat;
}

bool ResourceIMAP::doOpen()
{
  // Ensure that there is a kmail running
  QString error;
  int result = KDCOPServiceStarter::self()->findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null, QString::null, &error, &mAppId );
  KMessageBox::sorry( 0, error );
  return ( result == 0 );
}

void ResourceIMAP::doClose()
{
  // Nothing to close
}

Ticket * ResourceIMAP::requestSaveTicket()
{
  DCOPClient* dcopClient = kapp->dcopClient();
  QByteArray returnData;
  QCString returnType;
  if ( !dcopClient->call( mAppId, "KMailIface",
                          "lockContactsFolder()", QByteArray(),
                          returnType, returnData, true ) ) {
    return 0;
  }
  Q_ASSERT( returnType == "bool" );
  QDataStream argIn( returnData, IO_ReadOnly );
  bool ok;
  argIn >> ok;

  if( !ok )
    return 0;
  else
    return createTicket( this );
}

void ResourceIMAP::releaseSaveTicket( Ticket* )
{
  kdDebug() << "NYI: void ResourceIMAP::releaseSaveTicket( Ticket* )\n";
}

bool ResourceIMAP::load()
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error during incidences(QString)\n";
    return false;
  }

  QStringList lst = mKMailIcalIfaceStub->incidences( "Contact" );
  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::load()\n";
    return false;
  }

  // TODO: Do something with this QStringList of vCards.

  KTempFile tempFile( QString::null, ".vcf" );
  // For loading, send a DCOP call off to KMail
  DCOPClient* dcopClient = kapp->dcopClient();
  QByteArray outgoingData;
  QDataStream outgoingStream( outgoingData, IO_WriteOnly );
  outgoingStream << tempFile.name();
  QByteArray returnData;
  QCString returnType;
  // Important; we need the synchronous call, even though we don't
  // expect a return value.
  if ( !dcopClient->call( mAppId, "KMailIface",
                          "requestAddresses(QString)", outgoingData,
                          returnType, returnData, true ) ) {
    qDebug( "DCOP call failed" );
    return false;
  }
    
  // Now parse the vCards in that file
  QFile file( tempFile.name() );
  if ( !file.open( IO_ReadOnly ) ) {
    qDebug( "+++Could not open temp file %s", tempFile.name().latin1() );
    return false;
  }
  qDebug( "+++Opened temp file %s", tempFile.name().latin1() );
    
  mFormat->loadAll( addressBook(), this, &file );
    
  tempFile.unlink();

  // reset list of deleted addressees
  mDeletedAddressees.clear();
    
  return true;
}

bool ResourceIMAP::save( Ticket* )
{
  // TODO: Is this really necessary? It should be kept up to date all the time

  // FormatPlugin only supports loading from a file, not from
  // memory, so we have to write to a temp file first. This is all
  // very uncool, but that's the price for reusing the vCard
  // parser. In the future, the FormatPlugin interface needs
  // changing big time.
  KTempFile tempFile( QString::null, ".vcf" );
  mFormat->saveAll( addressBook(), this, tempFile.file() );
  tempFile.close();

  DCOPClient* dcopClient = kapp->dcopClient();
  QCString returnType;
  QByteArray returnData;
  QByteArray paramData;
  QDataStream paramStream( paramData, IO_WriteOnly );
  paramStream << tempFile.name();
  paramStream << mDeletedAddressees;
  if ( !dcopClient->call( mAppId, "KMailIface",
                          "storeAddresses(QString,QStringList)", paramData,
                          returnType, returnData, true ) )
    return false; // No need to continue in this case.
  Q_ASSERT( returnType == "bool" );
  QDataStream argIn( returnData, IO_ReadOnly );
  bool ok;
  argIn >> ok;
  mDeletedAddressees.clear();
  tempFile.unlink();
    
  // Always try to unlock
  if ( !dcopClient->call( mAppId, "KMailIface",
                          "unlockContactsFolder()", QByteArray(),
                          returnType, returnData, true ) ) {
    return false;
  }
  Q_ASSERT( returnType == "bool" );
  QDataStream argIn2( returnData, IO_ReadOnly );
  bool ok2;
  argIn2 >> ok2;
  return ( ok2 && ok );
}

bool ResourceIMAP::asyncLoad()
{
  // TODO: Implement this
  kdDebug() << "NYI: bool ResourceIMAP::asyncLoad()\n";
  return false;
}

bool ResourceIMAP::asyncSave( Ticket *ticket ) {
  // TODO: Implement this
  Q_UNUSED( ticket );
  kdDebug() << "NYI: bool ResourceIMAP::asyncSave( Ticket *ticket )\n";
  return false;
}

void ResourceIMAP::insertAddressee( const Addressee& addr )
{
  kdDebug(5800) << "ResourceIMAP::removeAddressee" << endl;

  // Call kmail ...
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during "
                << "ResourceIMAP::removeAddressee(const Addressee& addr)\n";
    } else {
      QString vCard;
      // TODO: Fill the vCard with the addr data
      mKMailIcalIfaceStub->addIncidence( "Contact", addr.uid(), vCard );
    }
  }
}

void ResourceIMAP::removeAddressee( const Addressee& addr )
{
  kdDebug(5800) << "ResourceIMAP::removeAddressee" << endl;

  // Call kmail ...
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during "
                << "ResourceIMAP::removeAddressee(const Addressee& addr)\n";
    } else {
      mKMailIcalIfaceStub->deleteIncidence( "Contact", addr.uid() );
    }
  }
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool ResourceIMAP::addIncidence( const QString& type, const QString& vCard )
{
  // kdDebug() << "ResourceIMAP::addIncidence( " << type << ", "
  //           << /*ical*/"..." << " )" << endl;

  bool silent = mSilent;
  mSilent = true;

  // TODO: add the vCard

  mSilent = false;

  return true;
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid )
{
  // kdDebug() << "ResourceIMAP::deleteIncidence( " << type << ", " << uid
  //           << " )" << endl;

  bool silent = mSilent;
  mSilent = true;

  // TODO: Remove the uid

  mSilent = false;
}

void ResourceIMAP::slotRefresh( const QString& type )
{
  bool silent = mSilent;
  mSilent = true;

  // TODO: Reload everything

  mSilent = false;
}

/*
 * KMail connection code
 */
bool ResourceIMAP::connectToKMail() const
{
  if ( !mKMailIcalIfaceStub ) {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->
      findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null,
                      QString::null, &error, &dcopService );
    if ( result != 0 ) {
      kdDebug(5800) << "Couldn't connect to the IMAP resource backend\n";
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

bool ResourceIMAP::connectKMailSignal( const QCString& signal,
                                       const QCString& method ) const
{
  ResourceIMAP* _this = const_cast<ResourceIMAP*>( this );
  return _this->connectDCOPSignal( "kmail", dcopObjectId, signal, method,
                                   false );
}

void ResourceIMAP::unregisteredFromDCOP( const QCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0;
  }
}


#include "resourceimap.moc"
