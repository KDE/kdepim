/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser                             *
*   geiseri@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "api_kdedevelopers.h"
#include "xmlrpciface.h"
#include <kdebug.h>

#include <qregexp.h>

kdedevelopersAPI::kdedevelopersAPI( const KURL &kurl, QObject *parent, const char *name ) : blogInterface( kurl, parent, name )
{
	m_xmlrpcServer = new KXMLRPC::Server( "", this );
	m_xmlrpcServer->setUrl( kurl );
	isValid = false;
}


kdedevelopersAPI::~kdedevelopersAPI()
{}

void kdedevelopersAPI::initServer()
{
	QStringList args;
	args << "C6CE3FFB3174106584CBB250C0B0519BF4E294"
	<< m_username
	<< m_password;

	m_xmlrpcServer->setUrl( serverURL );

	m_xmlrpcServer->call( "blogger.getUserInfo", args,
	                      this, SLOT( userInfoFinished( const QValueList<QVariant> & ) ),
	                      this, SLOT( fault( int, const QString& ) ) );
}

void kdedevelopersAPI::getBlogs()
{
	if ( isValid )
	{
		kdDebug() << "Fetch Blogs..." << endl;
		QValueList<QVariant> args;
		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" )
		<< QVariant( m_username )
		<< QVariant( m_password );

		m_xmlrpcServer->call( "blogger.getUsersBlogs", args,
		                      this, SLOT( blogListFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );
	} else {
          kdDebug() << "kdedevelopersAPI::getBlogs(): Server not yet initialized. Please wait until userInfoFinished is emited"<<endl;
        }

}

void kdedevelopersAPI::post( const blogPost &pst, bool publish )
{
	if ( isValid )
	{
		QValueList<QVariant> args;
		QString content = pst.content;

		QString post = "";
		post += m_template.titleTagOpen + pst.postInfo.title + m_template.titleTagClose + m_template.categoryTagOpen + pst.category + m_template.categoryTagClose;
		post += "\n";
		post += content;

		post = post.replace( QRegExp( "&" ), "&amp;" );
		post = post.replace( QRegExp( "<" ), "&lt;" );
		post = post.replace( QRegExp( ">" ), "&gt;" );
		post = post.replace( QRegExp( "\"" ), "&quot;" );

		kdDebug() << "Posting data..." << endl;

		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" );

		args << QVariant( pst.postInfo.blogID );
		args << QVariant( m_username )
		<< QVariant( m_password )
		<< QVariant( pst.content )
		<< QVariant( publish );


		m_xmlrpcServer->call( "blogger.newPost", args,
		                      this, SLOT( postFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );
	}
}


void kdedevelopersAPI::fetchPosts( const QString &blogID, int maxPosts )
{
	if ( isValid )
	{
		kdDebug() << "Fetch Posts..." << endl;
		QValueList<QVariant> args;
		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" )
		<< QVariant( blogID )
		<< QVariant( m_username )
		<< QVariant( m_password )
		<< QVariant( maxPosts );

		m_xmlrpcServer->call( "blogger.getRecentPosts", args,
		                      this, SLOT( listFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );
	}
}

void kdedevelopersAPI::fetchPost( const QString &postID )
{
	if ( isValid )
	{
		kdDebug() << "Fetch Post..." << endl;
		QValueList<QVariant> args;
		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" )
		<< QVariant( postID )
		<< QVariant( m_username )
		<< QVariant( m_password );

		m_xmlrpcServer->call( "blogger.getPost", args,
		                      this, SLOT( getFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );
	}

}

// void kdedevelopersAPI::fetchTemplates(const QString &username, const QString &password)
//{
//}

void kdedevelopersAPI::editPost( const blogPost& pst, bool publish )
{
	if ( isValid )
	{
		QValueList<QVariant> args;

		QString post = "";
		post += m_template.titleTagOpen + pst.postInfo.title + m_template.titleTagClose + m_template.categoryTagOpen + pst.category + m_template.categoryTagClose;
		post += "\n";
		post += pst.content;

		post = post.replace( QRegExp( "&" ), "&amp;" );
		post = post.replace( QRegExp( "<" ), "&lt;" );
		post = post.replace( QRegExp( ">" ), "&gt;" );
		post = post.replace( QRegExp( "\"" ), "&quot;" );

		kdDebug() << "Posting data..." << endl;

		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" );

		args << QVariant( pst.postInfo.postID );

		args << QVariant( m_username )
		<< QVariant( m_password )
		<< QVariant( pst.content )
		<< QVariant( publish );

		m_xmlrpcServer->call( "blogger.editPost", args,
		                      this, SLOT( postFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );

	}
}

void kdedevelopersAPI::deletePost( const QString &postID )
{
	if ( isValid )
	{
		kdDebug() << "delete post..." << endl;
		QValueList<QVariant> args;
		args << QVariant( "C6CE3FFB3174106584CBB250C0B0519BF4E294" )
		<< QVariant( postID )
		<< QVariant( m_username )
		<< QVariant( m_password )
		<< QVariant( true );

		m_xmlrpcServer->call( "blogger.getRecentPosts", args,
		                      this, SLOT( deleteFinished( const QValueList<QVariant> & ) ),
		                      this, SLOT( fault( int, const QString& ) ) );
	}
}

void kdedevelopersAPI::userInfoFinished( const QValueList<QVariant> &message )
{
	isValid = true;
	kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
	const QValueList<QVariant> posts = message;
	QValueList<QVariant>::ConstIterator it = posts.begin();
	QValueList<QVariant>::ConstIterator end = posts.end();
	for ( ; it != end; ++it )
	{
		kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
		const QMap<QString, QVariant> postInfo = ( *it ).toMap();
		const QString nickname = postInfo[ "nickname" ].toString();
		const QString userid = postInfo[ "userid" ].toString();
		const QString email = postInfo[ "email" ].toString();
		kdDebug() << "Post " << nickname << " " << userid << " " << email << endl;
		emit serverInfo( nickname, userid, email );
	}
}

void kdedevelopersAPI::dumpBlog( const blogPost &blog ) 
{
  kdDebug() << "-----------------------------------" << endl;
  kdDebug() << "Post " << blog.postInfo.postID << " by \"" << 
               blog.postInfo.userID << "\" on " << 
               blog.postInfo.date.toString() << endl;
  kdDebug() << "Title: " << blog.postInfo.title << endl;
  kdDebug() << blog.postInfo.description <<endl;
  kdDebug() << "-----------------------------------" << endl;
}
void kdedevelopersAPI::listFinished( const QValueList<QVariant> &message )
{
	//array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
	kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
	QStringList blogs;
        QValueList<blogPost> fullblogs;
        
	const QValueList<QVariant> posts = message[ 0 ].toList();
	QValueList<QVariant>::ConstIterator it = posts.begin();
	QValueList<QVariant>::ConstIterator end = posts.end();
	for ( ; it != end; ++it )
	{
		blogPost blog;
		kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
		const QMap<QString, QVariant> postInfo = ( *it ).toMap();
QStringList mapkeys = postInfo.keys();
kdDebug()<<endl<<endl<<"Keys: "<<mapkeys.join(", ")<<endl<<endl<<endl;


		const QDateTime creationdate = postInfo[ "dateCreated" ].toDateTime();
kdDebug()<<"dateCreated as string: "<<postInfo[ "dateCreated" ].toString()<<", and as QDateTime:"<<postInfo[ "dateCreated" ].toDateTime().toString()<< endl;
                blog.postInfo.date = creationdate;
		blog.postInfo.userID = postInfo[ "userid" ].toString();
		blog.postInfo.postID = postInfo[ "postid" ].toString();
		blog.content = postInfo[ "content" ].toString();
                blog.postInfo.title = postInfo[ "title" ].toString();
                blog.postInfo.description = postInfo[ "description" ].toString();
                
                dumpBlog( blog );
                
		blogs.append( blog.postInfo.postID );
                fullblogs.append( blog );                
	}

	emit recentPosts( blogs );
        emit recentPosts( fullblogs );
}
void kdedevelopersAPI::blogListFinished( const QValueList<QVariant> &message )
{
	kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
	QValueList<blogListItem> blogs;
	const QValueList<QVariant> posts = message[ 0 ].toList();
	QValueList<QVariant>::ConstIterator it = posts.begin();
	QValueList<QVariant>::ConstIterator end = posts.end();
	for ( ; it != end; ++it )
	{
		kdDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
		const QMap<QString, QVariant> postInfo = ( *it ).toMap();

		const QString blogID = postInfo[ "blogid" ].toString();
		const QString blogName = postInfo[ "blogName" ].toString();
		const QString blogURL = postInfo[ "url" ].toString();
		kdDebug() << "Message: " << blogID << " " << blogName << " " << blogURL << endl;
		blogListItem blog;
		blog.ID = blogID;
		blog.Name = blogName;
		blog.URL = blogURL;
		blogs.append( blog );
	}
	emit blogList( blogs );
}
void kdedevelopersAPI::deleteFinished( const QValueList<QVariant> & /*message */ )
{}
void kdedevelopersAPI::getFinished( const QValueList<QVariant> &message )
{
	kdDebug () << "TOP: " << message[ 0 ].typeName() << endl;
	const QMap<QString, QVariant> postInfo = message[ 0 ].toMap();
	const QDateTime date = postInfo[ "dateCreated" ].toDateTime();
	const QString userID = postInfo[ "userid" ].toString();
	const QString postID = postInfo[ "postid" ].toString();
	QString postContent = postInfo[ "content" ].toString();
	kdDebug() << "Post " << postID << " " << date.toString() << " " << userID << endl;

	blogPost post;
	post.postInfo.userID = userID;
	post.postInfo.date = date;
	post.postInfo.postID = postID;


	QString catTagOpen = m_template.categoryTagOpen;
	QString catTagClose = m_template.categoryTagClose;
	QString titleTagOpen = m_template.titleTagOpen;
	QString titleTagClose = m_template.titleTagClose;

	int catStart = postContent.find( catTagOpen, 0, false ) + catTagOpen.length();
	int catEnd = postContent.find( catTagClose, 0, false );
	if ( catEnd > catStart )
	{
		QString cat = postContent.mid( catStart, catEnd - catStart );
		postContent = postContent.remove( catStart - catTagOpen.length(), catEnd - catStart + catTagClose.length() + catTagOpen.length() );
		post.category = cat;
	}
	int titleStart = postContent.find( titleTagOpen, 0, false ) + titleTagOpen.length();
	int titleEnd = postContent.find( titleTagClose, 0, false );
	kdDebug() << titleStart << titleEnd << endl;
	if ( titleEnd > titleStart )
	{
		QString title = postContent.mid( titleStart, titleEnd - titleStart );
		postContent = postContent.remove( titleStart - titleTagOpen.length(), titleEnd - titleStart + titleTagClose.length() + titleTagOpen.length() );
		post.postInfo.title = title;
	}
	post.content = postContent;

	emit newPost( post );
}
void kdedevelopersAPI::fault( int code, const QString &message )
{
	kdDebug() << "Error " << code << " " << message << endl;
}
void kdedevelopersAPI::postFinished( const QValueList<QVariant> &/*data */)
{

	kdDebug() << "Post successful. " << endl;
}
#include "api_kdedevelopers.moc"


