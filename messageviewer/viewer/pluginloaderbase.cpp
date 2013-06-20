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



#include "pluginloaderbase.h"

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KLibrary>
#include <KLocale>
#include <KStandardDirs>

#include <QFile>
#include <QStringList>

PluginLoaderBase::PluginLoaderBase() : d(0) {}
PluginLoaderBase::~PluginLoaderBase() {}

QStringList PluginLoaderBase::types() const {
  QStringList result;
  QMap< QString, PluginMetaData >::const_iterator end( mPluginMap.constEnd() );
  for ( QMap< QString, PluginMetaData >::const_iterator it = mPluginMap.constBegin(); it != end ; ++it )
    result.push_back( it.key() );
  return result;
}

const PluginMetaData * PluginLoaderBase::infoForName( const QString & type ) const {
  return mPluginMap.contains( type ) ? &(const_cast<PluginLoaderBase*>(this)->mPluginMap[type]) : 0 ;
}


void PluginLoaderBase::doScan( const char * path ) {
  mPluginMap.clear();

  const QStringList list =
    KGlobal::dirs()->findAllResources( "data", path,
                                       KStandardDirs::Recursive |
                                       KStandardDirs::NoDuplicates );
  for ( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it ) {
    KConfig config( *it, KConfig::SimpleConfig);
    if ( config.hasGroup( "Misc" ) && config.hasGroup( "Plugin" ) ) {
      KConfigGroup group( &config, "Plugin" );

      const QString type = group.readEntry( "Type" ).toLower();
      if ( type.isEmpty() ) {
        kWarning() << "missing or empty [Plugin]Type value in \"" << *it << "\" - skipping";
        continue;
      }

      const QString library = group.readEntry( "X-KDE-Library" );
      if ( library.isEmpty() ) {
        kWarning() << "missing or empty [Plugin]X-KDE-Library value in \"" << *it << "\" - skipping";
        continue;
      }

      KConfigGroup group2( &config, "Misc" );

      QString name = group2.readEntry( "Name" );
      if ( name.isEmpty() ) {
        kWarning() << "missing or empty [Misc]Name value in \"" << *it << "\" - inserting default name";
        name = i18n("Unnamed plugin");
      }

      QString comment = group2.readEntry( "Comment" );
      if ( comment.isEmpty() ) {
        kWarning() << "missing or empty [Misc]Comment value in \"" << *it << "\" - inserting default name";
        comment = i18n("No description available");
      }

      mPluginMap.insert( type, PluginMetaData( library, name, comment ) );
    } else {
      kWarning() << "Desktop file \"" << *it << "\" doesn't seem to describe a plugin " << "(misses Misc and/or Plugin group)";
    }
  }
}

KLibrary::void_function_ptr PluginLoaderBase::mainFunc( const QString & type, const char * mf_name ) const {
  if ( type.isEmpty() || !mPluginMap.contains( type ) )
    return 0;

  const QString libName = mPluginMap[ type ].library;
  if ( libName.isEmpty() )
    return 0;

  const KLibrary * lib = openLibrary( libName );
  if ( !lib )
    return 0;

  PluginMetaData pmd = mPluginMap.value( type );
  pmd.loaded = true;
  mPluginMap[ type ] = pmd;

  const QString factory_name = libName + '_' + mf_name;
  KLibrary::void_function_ptr sym = const_cast<KLibrary*>( lib )->resolveFunction( factory_name.toLatin1() );
  if ( !sym ) {
    kWarning() << "No symbol named \"" << factory_name.toLatin1() << "\" (" << factory_name << ") was found in library \"" << libName << "\"";
    return 0;
  }

  return sym;
}

const KLibrary * PluginLoaderBase::openLibrary( const QString & libName ) const {
  KLibrary * library = new KLibrary( libName );
  if ( library->fileName().isEmpty() || !library->load() ) {
    kWarning() << "Could not load plugin library" << libName << "error:" << library->errorString() << library->fileName();
    delete library;
    return 0;
  }

  return library;
}
