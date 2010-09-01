/* -*- mode: C++; c-file-style: "gnu" -*-
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <config.h>
#include <unistd.h>

#include "kmaddrbook.h"
#include "kcursorsaver.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <dcopref.h>

#include <tqregexp.h>

void KabcBridge::addresses(TQStringList& result) // includes lists
{
  KCursorSaver busy(KBusyPtr::busy()); // loading might take a while

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  KABC::AddressBook::ConstIterator it;
  for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    const TQStringList emails = (*it).emails();
    TQString n = (*it).prefix() + " " +
		(*it).givenName() + " " +
		(*it).additionalName() + " " +
	        (*it).familyName() + " " +
		(*it).suffix();
    n = n.simplifyWhiteSpace();

    TQRegExp needQuotes("[^ 0-9A-Za-z\\x0080-\\xFFFF]");
    TQString endQuote = "\" ";
    TQStringList::ConstIterator mit;
    TQString addr, email;

    for ( mit = emails.begin(); mit != emails.end(); ++mit ) {
      email = *mit;
      if (!email.isEmpty()) {
	if (n.isEmpty() || (email.find( '<' ) != -1))
	  addr = TQString::null;
	else { // do we really need quotes around this name ?
          if (n.find(needQuotes) != -1)
	    addr = '"' + n + endQuote;
	  else
	    addr = n + ' ';
	}

	if (!addr.isEmpty() && (email.find( '<' ) == -1)
	    && (email.find( '>' ) == -1)
	    && (email.find( ',' ) == -1))
	  addr += '<' + email + '>';
	else
	  addr += email;
	addr = addr.stripWhiteSpace();
	result.append( addr );
      }
    }
  }
  KABC::DistributionListManager manager( addressBook );
  manager.load();
  result += manager.listNames();

  result.sort();
}

TQStringList KabcBridge::addresses()
{
    TQStringList entries;
    KABC::AddressBook::ConstIterator it;

    const KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
    for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
        entries += (*it).fullEmail();
    }
    return entries;
}

//-----------------------------------------------------------------------------
TQString KabcBridge::expandNickName( const TQString& nickName )
{
  if ( nickName.isEmpty() )
    return TQString::null;

  const TQString lowerNickName = nickName.lower();
  const KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  for( KABC::AddressBook::ConstIterator it = addressBook->begin();
       it != addressBook->end(); ++it ) {
    if ( (*it).nickName().lower() == lowerNickName )
      return (*it).fullEmail();
  }
  return TQString::null;
}


//-----------------------------------------------------------------------------

TQStringList KabcBridge::categories()
{
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  KABC::Addressee::List addresses = addressBook->allAddressees();
  TQStringList allcategories, aux;

  for ( KABC::Addressee::List::Iterator it = addresses.begin();
        it != addresses.end(); ++it ) {
    aux = ( *it ).categories();
    for ( TQStringList::ConstIterator itAux = aux.begin();
          itAux != aux.end(); ++itAux ) {
      // don't have duplicates in allcategories
      if ( allcategories.find( *itAux ) == allcategories.end() )
        allcategories += *itAux;
    }
  }
  return allcategories;
}
