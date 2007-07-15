/*
  This file is part of the blog resource.

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
class ResourceBlog : public ResourceCached
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
    ResourceBlog( const KConfigGroup &group );

    /**
      Create the blog resource from a XML-RPC access URL.

      @param url The URL used for XML-RPC access.
    */
    ResourceBlog( const KUrl &url );

    /**
      Destroy the blog resource.
    */
    ~ResourceBlog();

    /**
      The available APIs for accessing blogs.
    */
    enum APIType {
      MetaWeblog, Blogger, Unknown
    };

    /**
      Convert the API type enumerator to a QString.
    */
    QString APITypeToQString( const APIType &type ) ;

    /**
      Convert the API type enumerator to a QString.
    */
    APIType QStringToAPIType( const QString &type ) ;

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
    void setUser( const QString & );

    /**
      Get the username for the blog's XML-RPC authentication.

      @return The username for the blog.
    */
    QString user() const;

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
    void setAPI( const APIType & );

    /**
      Get name of the XML-RPC API used to access the blog.

      @return The enumeration of the chosen API.
    */
    APIType API() const;

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
      Posts a journal to the blog.

      @param key The journal to post.
      @return The success of the journal addition.
    */
    bool addJournal( Journal *journal );

    // Posts cannot be deleted from the server.
    bool deleteJournal( Journal *journal )
    {
      Q_UNUSED( journal )
      return false;
    }

    // The blog resource only handles journals.
    bool addEvent( Event *anEvent )
    {
      Q_UNUSED( anEvent )
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
    bool addTodo( Todo *todo )
    {
      Q_UNUSED( todo )
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

  protected Q_SLOTS:
    /**
      Converts a listed posting to a journal and adds to the cached resource.

      @param blogPosting A posting from the blog.
    */
    void slotListedPosting( KBlog::BlogPosting &blogPosting );

    /**
      Cleans up after the posting's listing completed.
    */
    void slotListPostingsFinished();

    /**
      Prints an error on a XML-RPC failure

      @param type The type of the error.
      @param errorMessage The specific cause of the error.
    */
    void slotError( const KBlog::APIBlog::errorType &type,
                    const QString &errorMessage );

  protected:
    /**
      Load the resource cache from the blog.

      @return The success of the blog retrieval operation.
    */
    bool doLoad( bool );

    /**
      Save the resource cache.

      @return The success of the cache save operation.
    */
    bool doSave( bool );

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
      The URL used for XML-RPC access.
    */
    KUrl mUrl;

    /**
      The username for the blog's XML-RPC authentication.
    */
    QString mUser;

    /**
      The password for the blog's XML-RPC authentication.
    */
    QString mPassword;

    /**
      The XML-RPC object used to access the blog.
    */
    KBlog::APIBlog *mAPI;

    /**
      The list of created journal objects.
    */
    Journal::List *mJournalList;

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
};

}

#endif
