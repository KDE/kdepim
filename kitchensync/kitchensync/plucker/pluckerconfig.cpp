/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include "pluckerconfig.h"


#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

static KStaticDeleter<KSPlucker::PluckerConfig> s_deleter;

namespace KSPlucker {

static PluckerConfig *s_config;

PluckerConfig::PluckerConfig()
{}

PluckerConfig::~PluckerConfig()
{}

PluckerConfig* PluckerConfig::self()
{
  if ( !s_config )
    s_deleter.setObject(s_config, new PluckerConfig() );

  return s_config;
}

QStringList PluckerConfig::pluckerFiles()const
{
  return m_paths;
}

QString     PluckerConfig::javaPath()const
{
  return m_javaPath;
}

QString     PluckerConfig::pluckerPath()const
{
  return m_pluckerPath;
}

QStringList PluckerConfig::konnectorIds()const
{
  return m_konnectors;
}

void PluckerConfig::setPluckerFiles( const QStringList& path )
{
  m_paths = path;
}

void PluckerConfig::setJavaPath( const QString& java)
{
  m_javaPath = java;
}

void PluckerConfig::setPluckerPath( const QString& pl)
{
  m_pluckerPath = pl;
}

void PluckerConfig::setKonnectorIds( const QStringList& lst )
{
  m_konnectors = lst;
}

void PluckerConfig::load(const QString& profileUid)
{
  KConfig config( locateLocal( "appdata", "plucker_config" ) );
  config.setGroup( profileUid );

  m_paths        = config.readPathListEntry( "PluckerFiles" );
  m_pluckerPath  = config.readPathEntry( "PluckerPath" );
  m_javaPath     = config.readPathEntry( "JavaPath" );
  m_konnectors   = config.readListEntry( "KonnectorIds" );

  kdDebug() << "Konnectors " << m_konnectors << " " << config.readEntry( "KonnectorsIds" ) << endl;
}

void PluckerConfig::save(const QString& profileUid)
{
  KConfig config( locateLocal( "appdata", "plucker_config" ) );
  config.setGroup( profileUid );

  config.writePathEntry( "PluckerFiles", m_paths );
  config.writePathEntry( "PluckerPath",  m_pluckerPath );
  config.writePathEntry( "JavaPath",     m_javaPath );
  config.writeEntry( "KonnectorIds",     m_konnectors );
}


}
