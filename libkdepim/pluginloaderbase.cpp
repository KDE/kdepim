/*  -*- c++ -*-
    This file is part of libkdepim.

    Copyright (c) 2002,2004 Marc Mutz <mutz@kde.org>

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
*/

#include <pluginloaderbase.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kglobal.h>
#include <kdebug.h>

#include <tqfile.h>
#include <tqstringlist.h>

static kdbgstream warning() {
  return kdWarning( 5300 ) << "PluginLoaderBase: ";
}
#ifndef NDEBUG
static kdbgstream debug( bool cond )
#else
static kndbgstream debug( bool cond )
#endif
{
  return kdDebug( cond, 5300 ) << "PluginLoaderBase: ";
}

namespace KPIM {

  PluginLoaderBase::PluginLoaderBase() : d(0) {}
  PluginLoaderBase::~PluginLoaderBase() {}


  TQStringList PluginLoaderBase::types() const {
    TQStringList result;
    for ( TQMap< TQString, PluginMetaData >::const_iterator it = mPluginMap.begin();
	  it != mPluginMap.end() ; ++it )
      result.push_back( it.key() );
    return result;
  }

  const PluginMetaData * PluginLoaderBase::infoForName( const TQString & type ) const {
    return mPluginMap.contains( type ) ? &(mPluginMap[type]) : 0 ;
  }


  void PluginLoaderBase::doScan( const char * path ) {
    mPluginMap.clear();

    const TQStringList list =
      KGlobal::dirs()->findAllResources( "data", path, true, true );
    for ( TQStringList::const_iterator it = list.begin() ;
	  it != list.end() ; ++it ) {
      KSimpleConfig config( *it, true );
      if ( config.hasGroup( "Misc" ) && config.hasGroup( "Plugin" ) ) {
	config.setGroup( "Plugin" );

	const TQString type = config.readEntry( "Type" ).lower();
	if ( type.isEmpty() ) {
	  warning() << "missing or empty [Plugin]Type value in \""
		    << *it << "\" - skipping" << endl;
	  continue;
	}

	const TQString library = config.readEntry( "X-KDE-Library" );
	if ( library.isEmpty() ) {
	  warning() << "missing or empty [Plugin]X-KDE-Library value in \""
		    << *it << "\" - skipping" << endl;
	  continue;
	}

	config.setGroup( "Misc" );

	TQString name = config.readEntry( "Name" );
	if ( name.isEmpty() ) {
	  warning() << "missing or empty [Misc]Name value in \""
		    << *it << "\" - inserting default name" << endl;
	  name = i18n("Unnamed plugin");
	}

	TQString comment = config.readEntry( "Comment" );
	if ( comment.isEmpty() ) {
	  warning() << "missing or empty [Misc]Comment value in \""
		    << *it << "\" - inserting default name" << endl;
	  comment = i18n("No description available");
	}

	mPluginMap.insert( type, PluginMetaData( library, name, comment ) );
      } else {
	warning() << "Desktop file \"" << *it
		  << "\" doesn't seem to describe a plugin "
		  << "(misses Misc and/or Plugin group)" << endl;
      }
    }
  }

  void * PluginLoaderBase::mainFunc( const TQString & type,
				     const char * mf_name ) const {
    if ( type.isEmpty() || !mPluginMap.contains( type ) )
      return 0;

    const TQString libName = mPluginMap[ type ].library;
    if ( libName.isEmpty() )
      return 0;

    const KLibrary * lib = openLibrary( libName );
    if ( !lib )
      return 0;

    mPluginMap[ type ].loaded = true;

    const TQString factory_name = libName + '_' + mf_name;
    if ( !lib->hasSymbol( factory_name.latin1() ) ) {
      warning() << "No symbol named \"" << factory_name.latin1() << "\" ("
		<< factory_name << ") was found in library \"" << libName
		<< "\"" << endl;
      return 0;
    }

    return lib->symbol( factory_name.latin1() );
  }

  const KLibrary * PluginLoaderBase::openLibrary( const TQString & libName ) const {

    const TQString path = KLibLoader::findLibrary( TQFile::encodeName( libName ) );

    if ( path.isEmpty() ) {
      warning() << "No plugin library named \"" << libName
		<< "\" was found!" << endl;
      return 0;
    }

    const KLibrary * library = KLibLoader::self()->library( TQFile::encodeName( path ) );

    debug( !library ) << "Could not load library '" << libName << "'" << endl;

    return library;
  }


} // namespace KMime
