/*
    This file is part of kdepim.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "bloggingglobals.h"

QString BloggingGlobals::mAppID = QString("20ffffffd7ffffffc5ffffffbdffffff87ffffffb72d39fffffffe5c4bffffffcfffffff80ffffffd4665cfffffff375ffffff88ffffff871a0cffffff8029");

QString BloggingGlobals::getFunctionName( blogFunctions type )
{
  switch ( type ) {
    case bloggerGetUserInfo:    return "blogger.getUserInfo";
    case bloggerGetUsersBlogs:  return "blogger.getUsersBlogs";
    case bloggerGetRecentPosts: return "blogger.getRecentPosts";
    case bloggerNewPost:        return "blogger.newPost";
    case bloggerEditPost:       return "blogger.editPost";
    case bloggerDeletePost:     return "blogger.deletePost";
    case bloggerGetPost:        return "blogger.getPost";
    case bloggerGetTemplate:    return "blogger.getTemplate";
    case bloggerSetTemplate:    return "blogger.setTemplate";
    default: return QString::null;
  }
}

QValueList<QVariant> BloggingGlobals::defaultArgs( const QString &user, const QString &pw, const QString &id )
{
  QValueList<QVariant> args;
  args << QVariant( mAppID );
  if ( !id.isNull() ) {
    args << QVariant( id );
  }
  args << QVariant( user )
       << QVariant( pw );
  return args;
}

