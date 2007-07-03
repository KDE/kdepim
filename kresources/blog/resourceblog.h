 /*
	This file is part of libkcal.

	Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
	Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_RESOURCEBLOGDIR_H
#define KCAL_RESOURCEBLOGDIR_H

#include "blog_export.h"
#include <kurl.h>
#include <kdirwatch.h>

#include <kcal/incidence.h>
#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>

#include <kcal/resourcecached.h>

#include <kblog/blog.h>

#include <kio/jobclasses.h>
#include <kio/jobuidelegate.h>

namespace KIO {
	class FileCopyJob;
	class Job;
	class JobUiDelegate;
}

namespace KPIM {
	class ProgressItem;
}

class KJob;
namespace KCal {

/**
  This class provides a journal stored on a blog.
*/
class ResourceBlog : public ResourceCached
{
	Q_OBJECT
	friend class ResourceBlogConfig;

	public:
	ResourceBlog();
	/**
		Create resource from configuration information stored in KConfig object.
	*/
	ResourceBlog( const KConfigGroup &group );
	/**
		Create blog resource.

		@param url URL used for XML-RPC access.
	*/
	ResourceBlog(const KUrl &url);
	virtual ~ResourceBlog();

	void readConfig(const KConfigGroup &group);
	void writeConfig(KConfigGroup &group);

	void setUrl(const KUrl &);
	KUrl url() const;

	void setUser(const QString &);
	QString user() const;

	void setPassword(const QString &);
	QString password() const;

	void setAPI(const QString &);
	QString API() const;

	void setUseProgressManager(bool useProgressManager);
	bool useProgressManager() const;

	void setUseCacheFile(bool useCacheFile);
	bool useCacheFile() const;

	KABC::Lock *lock();

	bool isSaving();

	void dump() const;

	bool setValue( const QString &key, const QString &value );

	bool addJournal( Journal* journal );

	bool deleteJournal( Journal* journal );

	Journal::List journals( const QDate& );

	Journal* journal( const QString& uid );

	bool addEvent(Event *anEvent);

	bool deleteEvent(Event *);

	void deleteAllEvents() {}

	bool addTodo(Todo *todo);

	bool deleteTodo(Todo *);

	void deleteAllTodos() {}

	protected Q_SLOTS:
	void slotLoadJobResult( KJob * );
	void slotSaveJobResult( KJob * );

	void slotPercent( KJob *, unsigned long percent );

	protected:
	bool doLoad( bool syncCache );
	bool doSave( bool syncCache );

	void addInfoText( QString & ) const;

	private:
	void init();

	KUrl mUrl;
	QString mUser;
	QString mPassword;
	KBlog::APIBlog* mAPI;

	bool mUseProgressManager;
	bool mUseCacheFile;

	KIO::FileCopyJob *mDownloadJob;
	KIO::FileCopyJob *mUploadJob;

	KPIM::ProgressItem *mProgress;

	Incidence::List mChangedIncidences;

	KABC::Lock *mLock;

	class Private;
	Private *d;
};

}

#endif
