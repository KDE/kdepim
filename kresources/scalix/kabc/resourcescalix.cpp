/*
    This file is part of the scalix resource - based on the kolab resource.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resourcescalix.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/observer.h>
#include <kio/uiserver_stub.h>
#include <kmainwindow.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <qobject.h>
#include <qtimer.h>
#include <qstring.h>
#include <qfile.h>
#include <qapplication.h>

#include <assert.h>

#include "contact.h"

using namespace Scalix;

class ScalixFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KABC::ResourceScalix( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

K_EXPORT_COMPONENT_FACTORY(kabc_scalix,ScalixFactory)

static const char* s_kmailContentsType = "Contact";
static const char* s_attachmentMimeTypeContact = "application/x-vnd.kolab.contact";
static const char* s_attachmentMimeTypeDistList = "application/x-vnd.kolab.contact.distlist";
static const char* s_inlineMimeType = "application/scalix-properties";

KABC::ResourceScalix::ResourceScalix( const KConfig *config )
  : KPIM::ResourceABC( config ),
    Scalix::ResourceScalixBase( "ResourceScalix-KABC" ),
    mCachedSubresource( QString::null ), mLocked( false )
{
  setType( "scalix" );
}

KABC::ResourceScalix::~ResourceScalix()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( isOpen() ) {
    close();
  }
}

void KABC::ResourceScalix::loadSubResourceConfig( KConfig& config,
                                                 const QString& name,
                                                 const QString& label,
                                                 bool writable )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  int completionWeight = group.readNumEntry( "CompletionWeight", 80 );
  mSubResources.insert( name, Scalix::SubResource( active, writable, label,
                                                   completionWeight ) );
}

bool KABC::ResourceScalix::doOpen()
{
  KConfig config( configFile() );

  // Read the calendar entries
  QValueList<KMailICalIface::SubResource> subResources;
  if ( !kmailSubresources( subResources, s_kmailContentsType ) )
    return false;
  mSubResources.clear();
  QValueList<KMailICalIface::SubResource>::ConstIterator it;
  for ( it = subResources.begin(); it != subResources.end(); ++it ) {
    loadSubResourceConfig( config, (*it).location, (*it).label, (*it).writable );
  }

  return true;
}

void KABC::ResourceScalix::doClose()
{
  KConfig config( configFile() );

  Scalix::ResourceMap::ConstIterator it;
  for ( it = mSubResources.begin(); it != mSubResources.end(); ++it ) {
    config.setGroup( it.key() );
    config.writeEntry( "Active", it.data().active() );
    config.writeEntry( "CompletionWeight", it.data().completionWeight() );
  }
}

KABC::Ticket * KABC::ResourceScalix::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdError() << "no addressbook" << endl;
    return 0;
  }
  mLocked = true;

  return createTicket( this );
}

void KABC::ResourceScalix::releaseSaveTicket( Ticket* ticket )
{
  mLocked = false;
  mCachedSubresource = QString::null;
  delete ticket;
}

QString KABC::ResourceScalix::loadContact( const QString& contactData,
                                          const QString& subResource,
                                          Q_UINT32 sernum,
                                          KMailICalIface::StorageFormat )
{
  KABC::Addressee addr = Contact::fromXml( contactData );

  addr.setResource( this );
  addr.setChanged( false );
  KABC::Resource::insertAddressee( addr ); // same as mAddrMap.insert( addr.uid(), addr );
  mUidMap[ addr.uid() ] = StorageReference( subResource, sernum );
  kdDebug(5650) << "Loaded contact uid=" << addr.uid() << " sernum=" << sernum << " fullName=" << addr.name() << endl;
  return addr.uid();
}

bool KABC::ResourceScalix::loadSubResource( const QString& subResource )
{
  bool scalixcontacts = loadSubResourceHelper( subResource, s_attachmentMimeTypeContact, KMailICalIface::StorageXML );
  bool scalixdistlists = loadSubResourceHelper( subResource, s_attachmentMimeTypeDistList, KMailICalIface::StorageXML );
  bool vcardstyle = loadSubResourceHelper( subResource, s_inlineMimeType, KMailICalIface::StorageIcalVcard );
  return scalixcontacts && scalixdistlists && vcardstyle;
}

bool KABC::ResourceScalix::loadSubResourceHelper( const QString& subResource,
                                                 const char* mimetype,
                                                 KMailICalIface::StorageFormat format )
{
  int count = 0;
  if ( !kmailIncidencesCount( count, mimetype, subResource ) ) {
    kdError() << "Communication problem in KABC::ResourceScalix::loadSubResourceHelper()\n";
    return false;
  }
  if ( !count )
    return true;

  // Read that many contacts at a time.
  // If this number is too small we lose time in kmail.
  // If it's too big the progressbar is jumpy.
  const int nbMessages = 200;

  (void)Observer::self(); // ensure kio_uiserver is running
  UIServer_stub uiserver( "kio_uiserver", "UIServer" );
  int progressId = 0;
  if ( count > 200 ) {
    progressId = uiserver.newJob( kapp->dcopClient()->appId(), true );
    uiserver.totalFiles( progressId, count );
    uiserver.infoMessage( progressId, i18n( "Loading contacts..." ) );
    uiserver.transferring( progressId, "Contacts" );
  }

  for ( int startIndex = 0; startIndex < count; startIndex += nbMessages ) {
    QMap<Q_UINT32, QString> lst;

    if ( !kmailIncidences( lst, mimetype, subResource, startIndex, nbMessages ) ) {
      kdError() << "Communication problem in ResourceScalix::load()\n";
      if ( progressId )
        uiserver.jobFinished( progressId );
      return false;
    }

    for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
      loadContact( it.data(), subResource, it.key(), format );
    }
    if ( progressId ) {
      uiserver.processedFiles( progressId, startIndex );
      uiserver.percent( progressId, 100 * startIndex / count );
    }
  }

  kdDebug(5650) << "Contacts scalix resource: got " << count << " contacts in " << subResource << endl;

  if ( progressId )
    uiserver.jobFinished( progressId );
  return true;
}

bool KABC::ResourceScalix::load()
{
  mUidMap.clear();
  mAddrMap.clear();

  bool rc = true;
  Scalix::ResourceMap::ConstIterator itR;
  for ( itR = mSubResources.begin(); itR != mSubResources.end(); ++itR ) {
    if ( !itR.data().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( itR.key() );
  }

  return rc;
}

bool KABC::ResourceScalix::save( Ticket* )
{
  bool rc = true;

  for( ConstIterator it = begin(); it != end(); ++it )
    if( (*it).changed() ) {
      rc &= kmailUpdateAddressee( *it );
    }

  if ( !rc )
    kdDebug(5650) << k_funcinfo << " failed." << endl;
  return rc;
}

namespace Scalix {
struct AttachmentList {
  QStringList attachmentURLs;
  QStringList attachmentNames;
  QStringList attachmentMimeTypes;
  QStringList deletedAttachments;
  QValueList<KTempFile *> tempFiles;

  void addAttachment( const QString& url, const QString& name, const QString& mimetype ) {
    attachmentURLs.append( url );
    attachmentNames.append( name );
    attachmentMimeTypes.append( mimetype );
  }

  void updatePictureAttachment( const QImage& image, const QString& name );
  void updateAttachment( const QByteArray& data, const QString& name, const char* mimetype );
};
} // namespace

void AttachmentList::updatePictureAttachment( const QImage& image, const QString& name )
{
  assert( !name.isEmpty() );
  if ( !image.isNull() ) {
    KTempFile* tempFile = new KTempFile;
    image.save( tempFile->file(), "PNG" );
    tempFile->close();
    KURL url;
    url.setPath( tempFile->name() );
    kdDebug(5650) << "picture saved to " << url.path() << endl;
    addAttachment( url.url(), name, "image/png" );
  } else {
    deletedAttachments.append( name );
  }
}

void AttachmentList::updateAttachment( const QByteArray& data, const QString& name, const char* mimetype )
{
  assert( !name.isEmpty() );
  if ( !data.isNull() ) {
    KTempFile* tempFile = new KTempFile;
    tempFile->file()->writeBlock( data );
    tempFile->close();
    KURL url;
    url.setPath( tempFile->name() );
    kdDebug(5650) << "data saved to " << url.path() << endl;
    addAttachment( url.url(), name, mimetype );
  } else {
    deletedAttachments.append( name );
  }
}

bool KABC::ResourceScalix::kmailUpdateAddressee( const Addressee& addr )
{
  const QString uid = addr.uid();
  QString subResource;
  Q_UINT32 sernum;
  if ( mUidMap.find( uid ) != mUidMap.end() ) {
    subResource = mUidMap[ uid ].resource();
    if ( !subresourceWritable( subResource ) ) {
      kdWarning() << "Wow! Something tried to update a non-writable addressee! Fix this caller: " << kdBacktrace() << endl;
      return false;
    }
    sernum = mUidMap[ uid ].serialNumber();
  } else {
    if ( !mCachedSubresource.isNull() ) {
      subResource = mCachedSubresource;
    } else {
      subResource = findWritableResource( mSubResources );
      // We were locked, remember the subresource we are working with until
      // we are unlocked
      if ( mLocked )
        mCachedSubresource = subResource;
    }
    if ( subResource.isEmpty() )
      return false;
    sernum = 0;
  }

  AttachmentList att;
  QString subject = addr.formattedName();

  QString mimetype = s_inlineMimeType;

  QString data = Contact::toXml( addr );

  CustomHeaderMap customHeaders;
  customHeaders.insert( "X-Scalix-Class", "IPM.Contact" );

  bool rc = kmailUpdate( subResource, sernum, data, mimetype, subject,
                         customHeaders,
                         att.attachmentURLs, att.attachmentMimeTypes, att.attachmentNames,
                         att.deletedAttachments );
  if ( !rc )
    kdDebug(5650) << "kmailUpdate returned false!" << endl;
  if ( rc ) {
    kdDebug(5650) << "kmailUpdate returned, now sernum=" << sernum << " for uid=" << uid << endl;
    mUidMap[ uid ] = StorageReference( subResource, sernum );
    // This is ugly, but it's faster than doing
    // mAddrMap.find(addr.uid()), which would give the same :-(
    // Reason for this: The Changed attribute of Addressee should
    // be mutable
    const_cast<Addressee&>(addr).setChanged( false );
  }

  for( QValueList<KTempFile *>::Iterator it = att.tempFiles.begin(); it != att.tempFiles.end(); ++it ) {
    (*it)->setAutoDelete( true );
    delete (*it);
  }
  return rc;
}

void KABC::ResourceScalix::insertAddressee( const Addressee& addr )
{
  const QString uid = addr.uid();
  //kdDebug(5650) << k_funcinfo << uid << endl;
  bool ok = false;
  if ( mUidMap.contains( uid ) ) {
    mUidsPendingUpdate.append( uid );
  } else {
    mUidsPendingAdding.append( uid );
  }

  ok = kmailUpdateAddressee( addr );

  if ( ok )
    Resource::insertAddressee( addr );
}

void KABC::ResourceScalix::removeAddressee( const Addressee& addr )
{
  const QString uid = addr.uid();
  if ( mUidMap.find( uid ) == mUidMap.end() ) return;
  //kdDebug(5650) << k_funcinfo << uid << endl;
  const QString resource = mUidMap[ uid ].resource();
  if ( !subresourceWritable( resource ) ) {
    kdWarning() << "Wow! Something tried to delete a non-writable addressee! Fix this caller: " << kdBacktrace() << endl;
    return;
  }
  /* The user told us to delete, tell KMail */
  kmailDeleteIncidence( resource,
                        mUidMap[ uid ].serialNumber() );
  mUidsPendingDeletion.append( uid );
  mUidMap.remove( uid );

  Resource::removeAddressee( addr );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KABC::ResourceScalix::fromKMailAddIncidence( const QString& type,
                                                 const QString& subResource,
                                                 Q_UINT32 sernum,
                                                 int format,
                                                 const QString& contactXML )
{
  // Check if this is a contact
  if( type != s_kmailContentsType || !subresourceActive( subResource ) )
    return false;

  // Load contact to find the UID
  const QString uid = loadContact( contactXML, subResource, sernum,
      ( KMailICalIface::StorageFormat )format );

  //kdDebug(5650) << k_funcinfo << uid << endl;

  // Emit "addressbook changed" if this comes from kmail and not from the GUI
  if ( !mUidsPendingAdding.contains( uid )
       && !mUidsPendingUpdate.contains( uid ) ) {
    addressBook()->emitAddressBookChanged();
  } else {
    mUidsPendingAdding.remove( uid );
    mUidsPendingUpdate.remove( uid );
  }

  return true;
}

void KABC::ResourceScalix::fromKMailDelIncidence( const QString& type,
                                                 const QString& subResource,
                                                 const QString& uid )
{
  // Check if this is a contact
  if( type != s_kmailContentsType || !subresourceActive( subResource ) )
    return;

  //kdDebug(5650) << k_funcinfo << uid << endl;

  // Can't be in both, by contract
  if ( mUidsPendingDeletion.contains( uid ) ) {
    mUidsPendingDeletion.remove( uid );
  } else if ( mUidsPendingUpdate.contains( uid ) ) {
    // It's good to know if was deleted, but we are waiting on a new one to
    // replace it, so let's just sit tight.
  } else {
    // We didn't trigger this, so KMail did, remove the reference to the uid
    mAddrMap.remove( uid );
    mUidMap.remove( uid );
    addressBook()->emitAddressBookChanged();
  }
}

void KABC::ResourceScalix::fromKMailRefresh( const QString& type,
                                            const QString& /*subResource*/ )
{
  // Check if this is a contact
  if( type != s_kmailContentsType ) return;

  //kdDebug(5650) << k_funcinfo << endl;

  load(); // ### should call loadSubResource(subResource) probably
  addressBook()->emitAddressBookChanged();
}

void KABC::ResourceScalix::fromKMailAddSubresource( const QString& type,
                                                   const QString& subResource,
                                                   const QString& label,
                                                   bool writable )
{
  if( type != s_kmailContentsType ) return;

  if ( mSubResources.contains( subResource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Contact" );
  loadSubResourceConfig( config, subResource, label, writable );
  loadSubResource( subResource );
  addressBook()->emitAddressBookChanged();
  emit signalSubresourceAdded( this, type, subResource );
}

void KABC::ResourceScalix::fromKMailDelSubresource( const QString& type,
                                                   const QString& subResource )
{
  if( type != s_kmailContentsType ) return;

  if ( !mSubResources.contains( subResource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mSubResources.erase( subResource );

  KConfig config( configFile() );
  config.deleteGroup( subResource );
  config.sync();

  // Make a list of all uids to remove
  Scalix::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    if ( mapIt.data().resource() == subResource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      mAddrMap.remove( *it );
      mUidMap.remove( *it );
    }

    addressBook()->emitAddressBookChanged();
  }

  emit signalSubresourceRemoved( this, type, subResource );
}



void KABC::ResourceScalix::fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                                    const QString& /* type */,
                                                    const QString& folder )
{
  // FIXME
  KMailICalIface::StorageFormat format = KMailICalIface::StorageXML;
  for( QMap<Q_UINT32, QString>::ConstIterator it = map.begin(); it != map.end(); ++it ) {
    loadContact( it.data(), folder, it.key(), format );
  }
  if ( !addressBook() ){
    kdDebug(5650) << "asyncLoadResult() : addressBook() returning NULL pointer.\n";
  }else
    addressBook()->emitAddressBookChanged();
}

QStringList KABC::ResourceScalix::subresources() const
{
  return mSubResources.keys();
}

bool KABC::ResourceScalix::subresourceActive( const QString& subresource ) const
{
  if ( mSubResources.contains( subresource ) ) {
    return mSubResources[ subresource ].active();
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

bool KABC::ResourceScalix::subresourceWritable( const QString& subresource ) const
{
  if ( mSubResources.contains( subresource ) ) {
    return mSubResources[ subresource ].writable();
  }
  return false; //better a safe default
}

int KABC::ResourceScalix::subresourceCompletionWeight( const QString& subresource ) const
{
  if ( mSubResources.contains( subresource ) ) {
    return mSubResources[ subresource ].completionWeight();
  }

  kdDebug(5650) << "subresourceCompletionWeight( " << subresource << " ): not found, using default\n";

  return 80;
}

QString KABC::ResourceScalix::subresourceLabel( const QString& subresource ) const
{
  if ( mSubResources.contains( subresource ) ) {
    return mSubResources[ subresource ].label();
  }

  kdDebug(5650) << "subresourceLabel( " << subresource << " ): not found!\n";
  return QString::null;
}

void KABC::ResourceScalix::setSubresourceCompletionWeight( const QString& subresource, int completionWeight )
{
  if ( mSubResources.contains( subresource ) ) {
    mSubResources[ subresource ].setCompletionWeight( completionWeight );
  } else {
    kdDebug(5650) << "setSubresourceCompletionWeight: subresource " << subresource << " not found" << endl;
  }
}

QMap<QString, QString> KABC::ResourceScalix::uidToResourceMap() const
{
  // TODO: Couldn't this be made simpler?
  QMap<QString, QString> map;
  Scalix::UidMap::ConstIterator mapIt;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    map[ mapIt.key() ] = mapIt.data().resource();
  return map;
}

void KABC::ResourceScalix::setSubresourceActive( const QString &subresource, bool active )
{
  if ( mSubResources.contains( subresource ) ) {
    mSubResources[ subresource ].setActive( active );
    load();
  } else {
    kdDebug(5650) << "setSubresourceCompletionWeight: subresource " << subresource << " not found" << endl;
  }
}

#include "resourcescalix.moc"
