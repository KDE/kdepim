/***************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser                             *
*   geiseri@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef BLOGINTERFACE_H
#define BLOGINTERFACE_H

#include <qobject.h>
#include <qdatetime.h>
#include <kurl.h>
/**
This is the main interface for blog backends

@author ian reinhart geiser
*/

class QString;



struct blogListItem
{
	QString ID;
	QString Name;
	QString URL;
};

struct blogPostItem
{
	QDateTime date;
	QString userID;
	QString blogID;
	QString postID;
	QString title;
        QString description;
};

struct blogPost
{
	blogPostItem postInfo;
	QString category;
	QString content;
};

struct blogTemplate
{
	QString categoryTagOpen;
	QString categoryTagClose;
	QString titleTagOpen;
	QString titleTagClose;
};

class blogInterface : public QObject
{
		Q_OBJECT
	public:
		blogInterface( const KURL &server, QObject *parent = 0L, const char *name = 0L );
		virtual ~blogInterface();

		void setPassword( const QString &pass );
		QString password();

		void setUsername( const QString &uname );
		QString username();

		void setURL( const KURL& url );
		KURL url();

		void setTemplateTags( const blogTemplate& Template );
		blogTemplate templateTags();


	public slots:
		virtual void initServer() = 0;
		virtual void getBlogs() = 0;
		virtual void post( const blogPost& data, bool publish = false ) = 0;
		virtual void editPost( const blogPost& data, bool publish = false ) = 0;
		virtual void fetchPosts( const QString &blogID, int maxPosts ) = 0;
		virtual void fetchPost( const QString &postID ) = 0;
		// void fetchTemplates() = 0;
		virtual void deletePost( const QString &postID ) = 0;

	signals:
		void serverInfo( const QString &/*nickname*/, const QString & /*m_userid*/, const QString & /*email*/ );
		void blogList( QValueList<blogListItem> /*blogs*/ );
		void recentPosts( QStringList /*postIDs*/ );
                void recentPosts( const QValueList<blogPost> &/*blogs*/ );
		//void post( const blogPost &post );
		void postFinished( bool /*success*/ );
		void publishFinished( bool /*success*/ );
		void editFinished( bool /*success*/ );
		void deleteFinished( bool /*success*/ );
		void newPost( const blogPost& /*post*/ );

		// Error message
		void error( const QString &/*faultMessage*/ );

	protected:
		KURL serverURL;
		blogTemplate m_template;
		QString m_password;
		QString m_username;
};

#endif
