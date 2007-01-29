/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kconfig.h>
#include <kdebug.h>
#include <kconfiggroup.h>

#include "kabprefs.h"

#include "filter.h"

Filter::Filter()
  : mName( QString() ), mMatchRule( Matching ), mEnabled( true ),
    mInternal( false ), mIsEmpty( true )
{
}

Filter::Filter( const QString &name )
  : mName( name ), mMatchRule( Matching ), mEnabled( true ),
    mInternal( false ), mIsEmpty( false )
{
}

Filter::~Filter()
{
}

void Filter::setName( const QString &name )
{
  mName = name;

  mIsEmpty = false;
}

const QString &Filter::name() const
{
  return mName;
}

bool Filter::isInternal() const
{
  return mInternal;
}

void Filter::apply( KABC::Addressee::List &addresseeList )
{
  KABC::Addressee::List::Iterator iter;
  for ( iter = addresseeList.begin(); iter != addresseeList.end(); ) {
    if ( filterAddressee( *iter ) )
      ++iter;
    else
      iter = addresseeList.erase( iter );
  }
}

bool Filter::filterAddressee( const KABC::Addressee &a ) const
{
  QStringList::ConstIterator iter;
  iter = mCategoryList.begin();
  // empty filter always matches

  if ( iter == mCategoryList.end() ) {
    if ( mMatchRule == Matching )
      return true;
    else {
      if ( a.categories().empty() )
        return true;
      else
        return false;
    }
  }

  for ( ; iter != mCategoryList.end(); ++iter ) {
    if ( a.hasCategory( *iter ) )
      return ( mMatchRule == Matching );
  }

  return !( mMatchRule == Matching );
}

void Filter::setEnabled( bool on )
{
  mEnabled = on;

  mIsEmpty = false;
}

bool Filter::isEnabled() const
{
  return mEnabled;
}

void Filter::setCategories( const QStringList &list )
{
  mCategoryList = list;

  mIsEmpty = false;
}

const QStringList &Filter::categories() const
{
  return mCategoryList;
}

void Filter::save( KConfigGroup &cfg )
{
  cfg.writeEntry( "Name", mName );
  cfg.writeEntry( "Enabled", mEnabled );
  cfg.writeEntry( "Categories", mCategoryList );
  cfg.writeEntry( "MatchRule", (int)mMatchRule );
}

void Filter::restore( KConfigGroup &cfg )
{
  mName = cfg.readEntry( "Name", "<internal error>" );
  mEnabled = cfg.readEntry( "Enabled", true );
  mCategoryList = cfg.readEntry( "Categories" , QStringList() );
  mMatchRule = (MatchRule)cfg.readEntry( "MatchRule", int(Matching) );

  mIsEmpty = false;
}

void Filter::save( KConfig *config, const QString &baseGroup, Filter::List &list )
{
  {
    KConfigGroup s( config, baseGroup );

    // remove the old filters
    uint count = s.readEntry( "Count", 0 );
    for ( uint i = 0; i < count; ++i )
      config->deleteGroup( QString( "%1_%2" ).arg( baseGroup ).arg( i ) );

  }

  int index = 0;
  Filter::List::Iterator iter;
  for ( iter = list.begin(); iter != list.end(); ++iter ) {
    if ( !(*iter).mInternal ) {
      KConfigGroup s( config, QString( "%1_%2" ).arg( baseGroup )
                                                     .arg( index ) );
      (*iter).save( s );
      index++;
    }
  }

  KConfigGroup s( config, baseGroup );
  s.writeEntry( "Count", index );
}

Filter::List Filter::restore( KConfig *config, const QString &baseGroup )
{
  Filter::List list;
  int count = 0;
  Filter f;

  {
    KConfigGroup s( config, baseGroup );
    count = s.readEntry( "Count", 0 );
  }

  for ( int i = 0; i < count; i++ ) {
    {
      KConfigGroup s( config, QString( "%1_%2" ).arg( baseGroup ).arg( i ) );
      f.restore( s );
    }

    list.append( f );
  }

  const QStringList cats = KABPrefs::instance()->customCategories();
  for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
    Filter filter;
    filter.mName = *it;
    filter.mEnabled = true;
    filter.mCategoryList = QStringList(*it);
    filter.mMatchRule = Matching;
    filter.mInternal = true;
    filter.mIsEmpty = false;
    list.append( filter );
  }

  return list;
}

void Filter::setMatchRule( MatchRule rule )
{
  mMatchRule = rule;

  mIsEmpty = false;
}

Filter::MatchRule Filter::matchRule() const
{
  return mMatchRule;
}

bool Filter::isEmpty() const
{
  return mIsEmpty;
}
