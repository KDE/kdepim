 /*
    This file is part of libkcal.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Based on the remote resource:
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCEBLOGGING_H
#define KCAL_RESOURCEBLOGGING_H

#include "bloginterface.h"

#include <libkdepim/progressmanager.h>
#include <libkcal/incidence.h>
#include <libkcal/resourcecached.h>

namespace KCal {

class Journal;

enum blogAPITypes {
  blogAPIBlogger = 1,
  blogAPIBlogger2,
  blogAPIFile,
  blogAPIDrupal,
  blogAPIKDEDevelopers,
  blogAPIUnknown
};

enum ConnectionStatus {
  blogConnecting = 1,
  blogConnected = 2,
  blogLoading = 4,
  blogSaving = 8
};

/**
  This class provides journals stored as blogs on a bloggin server.
*/
class ResourceBlogging : public ResourceCached
{
    Q_OBJECT

    friend class ResourceBloggingConfig;

  public:
    /**
      Create resource from configuration information stored in KConfig object.
    */
    ResourceBlogging( const KConfig * );
    virtual ~ResourceBlogging();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );
    
    void setURL( const KURL & );
    KURL url() const;
    
    void setServerAPI( int );
    int serverAPI() const;
    
    void setTitleOpen( const QString & );
    QString titleOpen() const;
    void setTitleClose( const QString & );
    QString titleClose() const;
    void setCategoryOpen( const QString & );
    QString categoryOpen() const;
    void setCategoryClose( const QString & );
    QString categoryClose() const;

    KABC::Lock *lock();

    void dump() const;
    
    bool addEvent( Event* ) { return false; }
    bool addTodo( Todo * ) { return false; }
    void deleteEvent( Event* ) {}
    void deleteTodo( Todo * ) {}
    
  protected slots:
    void serverInfo( const QString &nickname, const QString & m_userid, const QString & email );
    void blogList( QValueList<blogListItem> blogs );
    void recentPosts( const QValueList<blogPost> &blogs );
    //void post( const blogPost &post );
    void postFinished( bool success );
    void publishFinished( bool success );
    void editFinished( bool success );
    void deleteFinished( bool success );
    void newPost( const blogPost& post );

    // Error message
    void error( const QString &faultMessage );

  protected:
    bool doLoad();
    bool doDownLoad();
    bool doSave();

    void addInfoText( QString & ) const;
    
    Journal *journalFromBlog( const blogPost &blog );
    blogPost blogFromJournal( Journal *journal );

  private:
    void init();
    blogInterface *initBlogAPI( int serverAPI, const KURL &url, QObject*parent );
    void decrementDownloadCount();
    void decrementUploadCount();
    void handleUploadResult( bool success );

    KURL mUrl;
    int mServerAPI;
    blogTemplate mTemplate;

    KPIM::ProgressItem *mProgress;

    KABC::Lock *mLock;
    blogInterface* mBlogInterface;
    static QString mAppID;
    int mConnectionStatus;

    QString mNick;
    QString mUserID;
    QString mEmail;
    int mDownloading;
    int mUploading;
    
    QValueList<blogListItem> mBlogList;

    class Private;
    Private *d;
};

}

#endif
