/**************************************************************************
*   Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include "api_metaweblog.h"

using namespace KBlog;

metaweblogAPI::metaweblogAPI( const KURL &kurl, QObject *parent, const char *name ) : bloggerAPI( kurl, parent, name )
{
}


metaweblogAPI::~metaweblogAPI()
{}


#include "api_metaweblog.moc"


