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

#include <kabc/formatfactory.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>

#include <qstring.h>

#include "resourceimap.h"
#include <kmessagebox.h>


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
  : KABC::Resource( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-KABC" ),
    mSilent( false )
{
  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( "vcard" );
}

KABC::ResourceIMAP::~ResourceIMAP()
{
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

  QStringList lst;
  if ( !kmailIncidences( lst, "Contact" ) ) {
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
  bool rc = kmailUpdate( "Contact", vCards );

  // Mark all of them as read
  for( Iterator it = begin(); it != end(); ++it )
    (*it).setChanged( false );

  return rc;
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

    QString vCard = mConverter.createVCard( addr );

    bool rc;
    if( !update )
      // Save the new addressee
      rc = kmailAddIncidence( "Contact", addr.uid(), vCard );
    else
      // Update existing addressee
      rc = kmailUpdate( "Contact", addr.uid(), vCard );

    if( rc )
        // This is ugly, but it's faster than doing
        // mAddrMap.find(addr.uid()), which would give the same :-(
        // Reason for this: The Changed attribute of Addressee should
        // be mutable
        const_cast<Addressee&>(addr).setChanged( false );
  }

  Resource::insertAddressee( addr );
}

void KABC::ResourceIMAP::removeAddressee( const Addressee& addr )
{
  if ( !mSilent )
    kmailDeleteIncidence( "Contact", addr.uid() );

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


#include "resourceimap.moc"
