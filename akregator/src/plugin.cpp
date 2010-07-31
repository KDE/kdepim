// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution

#include "plugin.h"


namespace Akregator {


Plugin::Plugin()
{}


Plugin::~Plugin()
{}


void
Plugin::addPluginProperty( const TQString& key, const TQString& value )
{
    m_properties[key.lower()] = value;
}


QString
Plugin::pluginProperty( const TQString& key )
{
    if ( m_properties.find( key.lower() ) == m_properties.end() )
        return "false";

    return m_properties[key.lower()];
}


bool
Plugin::hasPluginProperty( const TQString& key )
{
    return m_properties.find( key.lower() ) != m_properties.end();
}

}
