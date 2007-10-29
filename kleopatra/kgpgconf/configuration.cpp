/* -*- mode: c++; c-basic-offset:4 -*-
    configuration.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "configuration.h"

#include <QStringList>

#include <cassert>

Config::Config()
{
}

Config::~Config()
{
    qDeleteAll( m_components );
}

QStringList Config::componentList() const
{
    return m_components.keys();
}

ConfigComponent* Config::component( const QString& name ) const
{
    return m_components[name];
}

void Config::addComponent( ConfigComponent* component )
{
    assert( component );
    if ( m_components.contains( component->name() ) )
        return;
    m_components[component->name()] = component;
}

ConfigComponent::ConfigComponent( const QString& name ) : m_name( name )
{
}

ConfigComponent::~ConfigComponent()
{
    qDeleteAll( m_groups );
}

QString ConfigComponent::name() const
{
    return m_name;
}

void ConfigComponent::setName( const QString& name )
{
    m_name = name;
}

QString ConfigComponent::description() const
{
    return m_description;
}

void ConfigComponent::setDescription( const QString& description )
{
    m_description = description;
}

QStringList ConfigComponent::groupList() const
{
    return m_groups.keys();
}

ConfigGroup* ConfigComponent::group( const QString& name ) const
{
    return m_groups[name];
}

void ConfigComponent::addGroup( ConfigGroup* group )
{
    assert( group );
    if ( m_groups.contains( group->name() ) )
        return;
    m_groups[group->name()] = group;
}


ConfigEntry* ConfigComponent::entry( const QString& name ) const
{
    if ( m_entries.contains( name ) )
        return m_entries[name];
    Q_FOREACH ( ConfigGroup* const i, m_groups )
    {
        if ( ConfigEntry* entry = i->entry( name ) )
        {
            m_entries[name] = entry;
            return entry;
        }
    }
    return 0;
}

ConfigGroup::ConfigGroup( const QString& name ) : m_name( name )
{
}

ConfigGroup::~ConfigGroup()
{
    qDeleteAll( m_entries );
}

QString ConfigGroup::name() const
{
    return m_name;
}

void ConfigGroup::setName( const QString& name )
{
    m_name = name;
}

QString ConfigGroup::description() const
{
    return m_description;
}

void ConfigGroup::setDescription( const QString& description )
{
    m_description = description;
}

QStringList ConfigGroup::entryList() const
{
    return m_entries.keys();
}

ConfigEntry* ConfigGroup::entry( const QString& name ) const
{
    return m_entries[name];
}

void ConfigGroup::addEntry( ConfigEntry* entry )
{
    assert( entry );
    if ( m_entries.contains( entry->name() ) )
        return;
    m_entries[entry->name()] = entry;
}

ConfigEntry::ConfigEntry( const QString& name ) : m_dirty( false ), m_name( name ), m_readOnly( false )
{
}

bool ConfigEntry::isDirty() const
{
    return m_dirty;
}

void ConfigEntry::unsetDirty()
{
    m_dirty = false;
}

QString ConfigEntry::name() const
{
    return m_name;
}

void ConfigEntry::setName( const QString& name )
{
    if ( m_name == name )
        return;
    m_name = name;
    m_dirty = true;
}

QString ConfigEntry::description() const
{
    return m_description;
}

void ConfigEntry::setDescription( const QString& desc )
{
    if ( m_description == desc )
        return;
    m_description = desc;
    m_dirty = true;
}

void ConfigEntry::setReadOnly( bool ro )
{
    if ( m_readOnly == ro )
        return;
    m_readOnly = ro;
    m_dirty = true;
}

bool ConfigEntry::isReadOnly() const
{
    return m_readOnly;
}
