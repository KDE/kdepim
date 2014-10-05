/*
 * Copyright (C) 2014  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KBLOG_BLOGGER_H
#define KBLOG_BLOGGER_H

#include <kblog/blog.h>

#include <QMap>
#include <KUrl>
namespace KGAPI2
{
class Job;
}

/**
  @file

  This file is part of the  for accessing Blog Servers from the
  Google's Blogger and BlogPost service and defines the Blogger
  class.

  @author Daniel Vrátil \<dvratil\@redhat.com\>
 */

namespace KBlog
{

class BloggerPrivate;

/**
  @brief
  A class that can be used for access to Google blogs. The blogspot.com and
  blogger.com accounts support native Blogger API v3.

  @code
  Blog *myblog = new Blogger("http://myblog.blogspot.com");
  myblog->setProfileId( "12345678" ); // can be fetched via fetchProfileId()
  myblog->setBlogId( "1" ); // can be caught by listBlogs()
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPost( post );
  @endcode

  @author Daniel Vrátil \<dvratil\@redhat.com\>
 */
class KBLOG_EXPORT Blogger : public KBlog::Blog
{
    Q_OBJECT
public:
    /**
      Create an object for Blogger
      @param server The blog URL
      @param parent The parent object, inherited from QObject.
    */
    explicit Blogger(const KUrl &server, QObject *parent = 0);

    /**
       Destructor.
     */
    virtual ~Blogger();

    /**
      Returns name of the service.
    */
    QString interfaceName() const;

    /**
      Authenticate this instance of Blogger against Google account.

      When no @p authData is provided, full authentication will take place, which
      includes popping up a dialog with Google Accounts Sign-On.

      @param authData Optinal authData received from authenticated(QMap\<QString,QString\>)
                      signal emitted after previous authenticate() call.
      @see authenticated(QMap\<QString,QString\>)
    */
    void authenticate(const QMap<QString, QString> &authData = QMap<QString, QString>());

    /**
      Sets Google OAuth application API key.

      @param apiKey API key to use for requests.
      @see setSecretKey(QString)
    */
    void setApiKey(const QString &apiKey);

    /**
      Sets Google OAuth application secret key.

      @param secretKey Secret key to use for requests.
      @see setApiKey(QString)
    */
    void setSecretKey(const QString &secretKey);

    /**
      List the blogs available for this authentication on the server.

      @see void listedBlogs( const QList\<QMap\<QString,QString\>\>& )
    */
    void listBlogs();

    /**
      List recent posts on the server. The status of the posts will be Fetched.
      @param number The number of posts to fetch. The order is newest first.

      @see     void listedPosts( const QList\<KBlog::BlogPost\>& )
      @see     void fetchPost( KBlog::BlogPost* )
      @see     BlogPost::Status
    */
    void listRecentPosts(int number);

    /**
      Fetch the Post with a specific id.
      @param post This is the post with its id set correctly.

      @see BlogPost::setPostId( const QString& )
      @see fetchedPost( KBlog::BlogPost *post )
    */
    void fetchPost(KBlog::BlogPost *post);

    /**
      Remove a post from the server.
      @param post This is the post with its id set correctly.

      @see BlogPost::setPostId( const QString& )
      @see removedPost( KBlog::BlogPost* )
    */
    void removePost(KBlog::BlogPost *post);

    /**
      Create a new post on server.
      @param post This is send to the server.

      @see createdPost( KBlog::BlogPost *post )
    */
    void createPost(KBlog::BlogPost *post);

    /**
      Modify a post on server.
      @param post This is used to send the modified post including the correct id.
    */
    void modifyPost(KBlog::BlogPost *post);

    /**
      List the comments available for this post on the server.
      @param post The post, which posts should be listed.

      @see void listedComments( KBlog::BlogPost*, const QList\<KBlog::BlogComment\>& )
    */
    void listComments(KBlog::BlogPost *post);

Q_SIGNALS:
    /**
      This signal is emitted when a list of comments has been fetched
      from the blogging server.
      @param post This is the corresponding post.
      @param comments The list of comments.

      @see listComments( KBlog::BlogPost* )
    */
    void listedComments(KBlog::BlogPost *post, const QList<BlogComment> &comments);

    /**
      This signal is emitted when a list of blogs has been fetched
      from the blogging server.
      @param blogsList The list of blogs.

      @see listBlogs()
    */
    void listedBlogs(const QList<QMap<QString, QString> > &blogs);

    /**
      This signal is emitted when authentication process started by
      call to authenticate() is finished.

      @param authData Opaque authentication data (contains OAuth tokens) that
                      application can store in some persistent storage (like KWallet)
                      and pass them to authenticate() call on next start.
    */
    void authenticated(const QMap<QString, QString> &authData);

private:
    Q_DECLARE_PRIVATE(Blogger);

    Q_PRIVATE_SLOT(d_func(), void _k_onAuthenticateFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onListBlogsFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onListRecentPostsFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onFetchPostFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onRemovePostFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onCreatePostFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onModifyPostFinished(KGAPI2::Job *))
    Q_PRIVATE_SLOT(d_func(), void _k_onListCommentsFinished(KGAPI2::Job *))
};
}

#endif // KBLOG_BLOGGER_H

