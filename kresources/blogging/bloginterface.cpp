/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser                             *
*   geiseri@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "bloginterface.h"

blogInterface::blogInterface( const KURL &url, QObject *parent, const char *name ) :
		QObject( parent, name ),
		serverURL( url )
{}


blogInterface::~blogInterface()
{}

void blogInterface::setPassword( const QString &pass )
{
	m_password = pass;
}
QString blogInterface::password()
{
	return m_password;
}

void blogInterface::setUsername( const QString &uname )
{
	m_username = uname;
}
QString blogInterface::username()
{
	return m_username;
}

void blogInterface::setURL( const KURL& url )
{
	serverURL = url;
}
KURL blogInterface::url()
{
	return serverURL;
}

void blogInterface::setTemplateTags( const blogTemplate& Template )
{
	m_template = Template;
}
blogTemplate blogInterface::templateTags()
{
	return m_template;
}
#include "bloginterface.moc"
