/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser <geiseri@kde.org>           *
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "bloggerwrapper.h"
#include "xmlrpciface.h"
#include <kdebug.h>
#include <klocale.h>

#include <qregexp.h>

using namespace KBlog;



namespace KBlog {


void BloggerPostingWrapper::deleteFinished( const QValueList<QVariant> & )
{
kdDebug()<<"BloggerPostingWrapper::deleteFinished SUCCESSFUL"<<endl;
  if ( mBlog ) {
    mBlog->wasDeleted( true );
  }
  emit deleteFinishedSignal( true );
  delete this;
}

void BloggerPostingWrapper::editFinished( const QValueList<QVariant> &message )
{
  bool success = message[0].toBool();
kdDebug()<<"BloggerPostingWrapper::postFinished SUCCESSFUL?, success = "<<success<<endl;
  if ( mBlog ) {
    mBlog->wasUploaded( success );
  }
  emit editFinishedSignal( success );
  delete this;
}
  
void BloggerPostingWrapper::postFinished( const QValueList<QVariant> &message )
{
  const QString postID = message[ 0 ].toString();
kdDebug()<<"BloggerPostingWrapper::newFinished SUCCESSFUL, PostID="<<postID<<endl;
  if ( mBlog ) {
    mBlog->setPostID( postID );
    mBlog->wasUploaded( true );
  }
  emit postFinishedSignal( true );
  delete this;
}

void BloggerPostingWrapper::deleteFault( int code, const QString& message )
{
  if ( mBlog ) {
    mBlog->wasDeleted( false );
    mBlog->error( code, message );
  }
  emit errorSignal( message );
  delete this;
}

void BloggerPostingWrapper::postFault( int code, const QString& message )
{
  if ( mBlog ) {
    mBlog->wasUploaded( false );
    mBlog->error( code, message );
  }
  emit errorSignal( message );
  delete this;
}

}

#include "bloggerwrapper.moc"


