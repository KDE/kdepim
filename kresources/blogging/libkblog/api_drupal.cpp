/**************************************************************************
*   Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include "api_drupal.h"

using namespace KBlog;

drupalAPI::drupalAPI( const KURL &kurl, QObject *parent, const char *name ) : bloggerAPI( kurl, parent, name )
{
}


drupalAPI::~drupalAPI()
{}

QString drupalAPI::getFunctionName( blogFunctions type )
{
  switch ( type ) {
    case bloggerGetUserInfo:    return "blogger.GetUserInfo";
    case bloggerGetUsersBlogs:  return "blogger.GetUsersBlogs";
    case bloggerGetRecentPosts: return "blogger.GetRecentPosts";
    case bloggerNewPost:        return "blogger.NewPost";
    case bloggerEditPost:       return "blogger.EditPost";
    case bloggerDeletePost:     return "blogger.DeletePost";
    case bloggerGetTemplate:    return "blogger.GetTemplate";
    case bloggerSetTemplate:    return "blogger.SetTemplate";
    default: return QString::null;
  }
}

// TODO: drupal.newBlog( userid, password, title, body) -> postID
// TODO: drupal.editBlog( userid, passwrd, title, body) -> success (bool)
// TODO: drupal.deleteBlog( userid, pword, postID ) -> success (bool)



#include "api_drupal.moc"


