/*
    KAbc2Mutt

    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kabc/distributionlist.h>
#include <kapplication.h>
#include <klocale.h>

#include <qregexp.h>

#include <iostream>

#include "kabc2mutt.h"

static std::ostream & operator<< ( std::ostream &os, const QString &s );

KABC2Mutt::KABC2Mutt( QObject *parent, const char *name )
  : QObject( parent, name ), mFormat( Aliases ),
    mIgnoreCase( false ), mAllAddresses( false ),
    mAlternateKeyFormat( false ),
    mAddressBook( 0 )
{
}

void KABC2Mutt::run()
{
  mAddressBook = KABC::StdAddressBook::self( true );
  KABC::StdAddressBook::setAutomaticSave( false );

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( loadingFinished() ) );
}

void KABC2Mutt::loadingFinished()
{
  // print addressees
  KABC::AddressBook::ConstIterator iaddr;
  for ( iaddr = mAddressBook->begin(); iaddr != mAddressBook->end(); ++iaddr ) {
    const QString name = (*iaddr).givenName() + ' ' + (*iaddr).familyName();
    if ( !mQuery.isEmpty() ) {
      bool match = (name.find(mQuery, 0, mIgnoreCase) > -1) ||
                   ((*iaddr).preferredEmail().find( mQuery, 0, mIgnoreCase ) > -1 );
      if ( !match )
        continue;
    }

    const QStringList &allAddresses = (*iaddr).emails();
    QStringList::const_iterator from, to;
    bool multiple = false;

    if ( mAllAddresses ) {
      // use all entries
      multiple = allAddresses.size() > 1;
      from = allAddresses.begin();
      to = allAddresses.end();
    } else {
      // use only the first entry, the one returned by preferredEmail()
      from = to = allAddresses.begin();  // start with empty list
      if ( to != allAddresses.end() )
        ++to;
    }

    size_t index = 0;
    if ( mFormat == Aliases ) {
      static const QChar space = QChar( ' ' );
      static const QChar underscore = QChar( '_' );

      QString key;
      if ( !mAlternateKeyFormat )
        key = (*iaddr).givenName().left( 3 ) + (*iaddr).familyName().left( 3 );
      else
        if ( !(*iaddr).familyName().isEmpty() )
          key = (*iaddr).givenName().left( 1 ).lower() +
                (*iaddr).familyName().lower().replace( space, underscore );
        else
          key = (*iaddr).givenName().lower().replace( space, underscore );

      while ( from != to ) {
        std::cout << "alias " << key;
        if ( index )
          std::cout << index;
        std::cout << '\t' << name << " <" << (*from) << '>' << std::endl;
        ++index;
        ++from;
      }

      if ( !(*iaddr).nickName().isEmpty() ) {
        std::cout << "alias "
                  << (*iaddr).nickName().lower().replace( space, underscore )
                  << '\t' << name << " <"
                  << (*iaddr).preferredEmail() << '>' << std::endl;
      }
    } else {
      while ( from != to ) {
        std::cout << (*from) << '\t' << name;
        if ( multiple ) {
          if ( index )
            std::cout << "\t#" << index;
          else
            std::cout << '\t' << i18n( "preferred" );
          ++index;
        }
        std::cout << std::endl;
        ++from;
      }
    }
  }

  // print all distribution lists
  KABC::DistributionListManager manager( mAddressBook );
  manager.load();

  QStringList dists = manager.listNames();
  for ( QStringList::Iterator iaddr = dists.begin(); iaddr != dists.end(); ++iaddr ) {
    KABC::DistributionList *list = manager.list( *iaddr );
    if ( list ) {
      if ( !mQuery.isEmpty() ) {
        bool match = ((*iaddr).find(mQuery) > -1);
        if ( !match )
          continue;
      }

      QStringList emails = list->emails();
      if ( emails.isEmpty() )
        continue;

      if ( mFormat == Aliases ) {
        std::cout << "alias " << (*iaddr).replace( QRegExp( " " ), "_" )
                  << '\t' << emails.join( "," ) << std::endl;
      } else {
        std::cout << emails.join( "," ) << '\t' << (*iaddr) << '\t' << std::endl;
      }
    }
  }

  kapp->quit();
}

static std::ostream & operator<< ( std::ostream &os, const QString &s )
{
  os << s.local8Bit().data();
  return os;
}

#include "kabc2mutt.moc"
