/*
  This file is part of the blog resource.

  Copyright (c) 2007-2008 Mike McQuaid <mike@mikemcquaid.com>

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

#include <KUrl>

#include <kcal/resourcecached.h>

#include <kblog/blog.h>

namespace KPIM
{
class ProgressItem;
}

namespace KCal
{

/**
  This class provides a journal stored on a blog.
*/
class KCAL_RESOURCEBLOG_EXPORT ResourceBlog : public ResourceCached
{
  Q_OBJECT
    friend class ResourceBlogConfig;

  public:
    /**
      Create the blog resource..
    */
    ResourceBlog();

    /**
      Create the blog resource from configuration information.

      @param group The configuration information.
    */
    explicit ResourceBlog( const KConfigGroup &group );

    /**
      Destroy the blog resource.
    */
    ~ResourceBlog();

    /**
      Read resource parameters from configuration information.

      @param group The configuration information.
    */
    void readConfig( const KConfigGroup &group );

    /**
      Write resource parameters to configuration information.

      @param group The configuration information.
    */
    void writeConfig( KConfigGroup &group );

    /**
      Set the URL used for XML-RPC access to the blog.
    */
    void setUrl( const KUrl & );

    /**
      Get the URL used for XML-RPC access to the blog.

      @return The URL used for XML-RPC access.
    */
    KUrl url() const;

    /**
      Set the username for the blog's XML-RPC authentication.
    */
    void setUsername( const QString & );

    /**
      Get the username for the blog's XML-RPC authentication.

      @return The username for the blog.
    */
    QString username() const;

    /**
      Set the password for the blog's XML-RPC authentication.
    */
    void setPassword( const QString & );

    /**
      Get the password for the blog's XML-RPC authentication.

      @return The password for the blog.
    */
    QString password() const;

    /**
      Set the XML-RPC API used to access the blog.
    */
    void setAPI( const QString & );

    /**
      Get name of the XML-RPC API used to access the blog.

      @return The enumeration of the chosen API.
    */
    QString API() const;

    /**
      Sets the number of posts to download when loading from the blog.
    */
    void setDownloadCount( int downloadCount );

    /**
      Gets the number of posts to download when loading from the blog.

      @return The number of posts to download.
    */
    int downloadCount() const;

    /**
      Set whether to display the progress of operations.

      @param useProgressManager Whether to display the progress of operations.
    */
    void setUseProgressManager( bool useProgressManager );

    /**
      Get whether the progress of operations are displayed.

      @return Whether the progress of operations are displayed.
    */
    bool useProgressManager() const;

    /**
      Set whether to use a cachefile to store old journals.

      @param useCacheFile Whether to use a cachefile.
    */
    void setUseCacheFile( bool useCacheFile );

    /**
      Get whether a cachefile is used to store old journals.

      @return Whether a cachefile is used.
    */
    bool useCacheFile() const;

    /**
      Get the status of the cachefile lock.

      @return Whether the cachefile is locked.
    */
    KABC::Lock *lock ();

    /**
      Print the resource parameters to the console.
    */
    void dump() const;

    /**
      Set thr resource's parameters.

      @param key The parameter to set.
      @param value The value to set the parameter to.
      @return The success of the parameter assignment.
    */
    bool setValue( const QString &key, const QString &value );

    /**
      Fetches the list of postable blogs.

      @return The success of the fetch call.
    */
    bool listBlogs();

    /**
      Returns the current blog to post to.

      @return A pair of the blog ID and the blog name.
    */
    QPair<QString, QString> blog() const;

    /**
      Updates the blog to post to.

      @param id The unique ID of the blog.
      @param name The name of the blog.
    */
    void setBlog( const QString &id, const QString &name );

    // The blog resource only handles journals.
    bool addEvent( Event * )
    {
      return false;
    }

    // The blog resource only handles journals.
    bool deleteEvent( Event * )
    {
      return false;
    }

    // The blog resource only handles journals.
    void deleteAllEvents()
    {}

    // The blog resource only handles journals.
    bool addTodo( Todo * )
    {
       return false;
    }

    // The blog resource only handles journals.
    bool deleteTodo( Todo * )
    {
      return false;
    }

    // The blog resource only handles journals.
    void deleteAllTodos()
    {}

  Q_SIGNALS:
    /**
      Signals an available blog(s) to post to.

      @param blogs A list of maps containing the blogs' IDs and descriptions.
    */
    void signalBlogInfoRetrieved( const QList<QMap<QString,QString> > &blogs );

  protected Q_SLOTS:
    /**
      Converts listed posts to journal entries and adds them to the cached
      resource.

      @param posts A list of the posts from the blog.
    */
    void slotListedPosts( const QList<KBlog::BlogPost> &posts );

    /**
      Prints an error on a XML-RPC failure.

      @param type The type of the error.
      @param errorMessage The specific cause of the error.
    */
    void slotError( const KBlog::Blog::ErrorType &type,
                    const QString &errorMessage );

    /**
    Prints an error on a XML-RPC failure involving a post.

    @param type The type of the error.
    @param errorMessage The specific cause of the error.
    @param post The relevant post.
    */
    void slotErrorPost( const KBlog::Blog::ErrorType &type,
                           const QString &errorMessage,
                           KBlog::BlogPost *post );

    /**
    Prints an error on a XML-RPC failure involving listing posts.

    @param type The type of the error.
    @param errorMessage The specific cause of the error.
    @param post The relevant post.
    */
    void slotErrorListPosts( const KBlog::Blog::ErrorType &type,
                             const QString &errorMessage,
                             KBlog::BlogPost *post );

    /**
    Prints an error on a XML-RPC failure involving media.

    @param type The type of the error.
    @param errorMessage The specific cause of the error.
    @param media The relevant post media.
    */
    void slotErrorMedia( const KBlog::Blog::ErrorType &type,
                         const QString &errorMessage,
                         KBlog::BlogMedia *media );

    /**
      Updates the latest stored post ID to the ID returned from the blog post
      creation/update/deletion operation.

    @param post The last blog post modified on the server.
    */
    void slotSavedPost( KBlog::BlogPost *post );

    /**
      Updates the local list of available blogs to post to.

      @param blogs A map containing the blogs' ID and description.
    */
    void slotBlogInfoRetrieved( const QList<QMap<QString,QString> > &blogs );

  protected:
    /**
      Load the resource cache from the blog.

      @return The success of the blog retrieval operation.
    */
    bool doLoad( bool syncCache );

    /**
      Save the resource cache.

      @return The success of the cache save operation.
    */
    bool doSave( bool syncCache );
    bool doSave( bool syncCache, Incidence *incidence );

    /**
      Add the URL and API of the blog to the string.
    */
    void addInfoText( QString & ) const;

  private:
    /**
      Initialise the blog resource.
    */
    void init();

    /**
      The latest known numerical post ID used on the server.
    */
    int mLastKnownPostID;

    /**
      The URL used for XML-RPC access.
    */
    KUrl mUrl;

    /**
      The username for the blog's XML-RPC authentication.
    */
    QString mUsername;

    /**
      The password for the blog's XML-RPC authentication.
    */
    QString mPassword;

    /**
      The XML-RPC object used to access the blog.
    */
    KBlog::Blog *mBlog;

    /**
      The unique ID of the blog to send posts to.
    */
    QString mBlogID;

    /**
      The name of the blog to send posts to.
    */
    QString mBlogName;

    /**
      The number of posts to download when loading from the blog.
    */
    int mDownloadCount;

    /**
      Whether the progress of operations are displayed.
    */
    bool mUseProgressManager;

    /**
      Whether a cachefile is used to store old journals.
    */
    bool mUseCacheFile;

    /**
      The object used to manage the progress of operations display.
    */
    KPIM::ProgressItem *mProgress;

    /**
      The cachefile lock.
    */
    KABC::Lock *mLock;

    /**
      A map of all the blog posts awaiting server responses.
    */
    QMap<QString,KBlog::BlogPost*> *mPostMap;
};

}

#endif
