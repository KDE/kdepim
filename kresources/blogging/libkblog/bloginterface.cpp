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

using namespace KBlog;

// That terribly long app key was generated at http://www.blogger.com/developers/api/1_docs/register.html
// for the "KDE-Pim libkcal blogging resource".
blogInterface::blogInterface( const KURL &url, QObject *parent, const char *name ) :
    QObject( parent, name ),
    mServerURL( url ), mAppID( "ffffffa65412fffffff42c2d452cffffff8e340b6b135337ffffffcfffffffd01affffffb9ffffffff26ffffffb61fffffffc2" )
{}

blogInterface::~blogInterface()
{}

void blogInterface::setPassword( const QString &pass )
{
  mPassword = pass;
}
QString blogInterface::password() const 
{
  return mPassword;
}

void blogInterface::setUsername( const QString &uname )
{
  mUsername = uname;
}
QString blogInterface::username() const 
{
  return mUsername;
}

void blogInterface::setURL( const KURL& url )
{
  mServerURL = url;
}
KURL blogInterface::url() const 
{
  return mServerURL;
}

void blogInterface::setTemplateTags( const BlogTemplate& Template )
{
  mTemplate = Template;
}
BlogTemplate blogInterface::templateTags() const 
{
  return mTemplate;
}
#include "bloginterface.moc"
