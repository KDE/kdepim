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

#include "blogger.h"
#include "blog_p.h"
#include <kblog/blogpost.h>
#include <kblog/blogcomment.h>

#include <KUrl>
#include <QDebug>
#include <KLocalizedString>

#include <KGAPI/Blogger/Blog>
#include <KGAPI/Blogger/BlogFetchJob>
#include <KGAPI/Blogger/Post>
#include <KGAPI/Blogger/PostCreateJob>
#include <KGAPI/Blogger/PostDeleteJob>
#include <KGAPI/Blogger/PostFetchJob>
#include <KGAPI/Blogger/PostModifyJob>
#include <KGAPI/Blogger/Comment>
#include <KGAPI/Blogger/CommentFetchJob>
#include <KGAPI/Account>
#include <KGAPI/AuthJob>

Q_DECLARE_METATYPE(KBlog::BlogPost*)
Q_DECLARE_METATYPE(KGAPI2::Job*)
typedef QMap<QString, QString> KBlogInfo;

#define KBLOGPOST_PROPERTY "KBlogPostProperty"
#define JOB_PROPERTY "JobProperty"

namespace KBlog
{

class BloggerPrivate: public KBlog::BlogPrivate
{
  public:
    BloggerPrivate(Blogger *parent);
    virtual ~BloggerPrivate();

    void updateKBlogPost(KBlog::BlogPost *kblog, const KGAPI2::Blogger::PostPtr &postPtr);
    KBlog::BlogPost KGAPIPostToKBlogPost(const KGAPI2::Blogger::PostPtr &postPtr);
    QList<KBlog::BlogPost> KGAPIPostsToKBlogPosts(const KGAPI2::ObjectsList &posts);
    KGAPI2::Blogger::PostPtr KBlogPostToKGAPI(const BlogPost * const kblog);

    KBlogInfo KGAPIBlogToKBlogBlog(const KGAPI2::Blogger::BlogPtr &blogPtr);
    QList<KBlogInfo> KGAPIBlogsToKBlogBlogs(const KGAPI2::ObjectsList &blogs);

    KBlog::BlogComment KGAPICommentToKBlogComment(const KGAPI2::Blogger::CommentPtr &commentPtr);
    QList<KBlog::BlogComment> KGAPICommentsToKBlogComments(const KGAPI2::ObjectsList &comments);

    bool handleError(KGAPI2::Job *job);

    void _k_onAuthenticateFinished(KGAPI2::Job *job);
    void _k_onListBlogsFinished(KGAPI2::Job *job);
    void _k_onListRecentPostsFinished(KGAPI2::Job *job);
    void _k_onFetchPostFinished(KGAPI2::Job *job);
    void _k_onCreatePostFinished(KGAPI2::Job *job);
    void _k_onRemovePostFinished(KGAPI2::Job *job);
    void _k_onModifyPostFinished(KGAPI2::Job *job);
    void _k_onListCommentsFinished(KGAPI2::Job *job);

    QString apiKey;
    QString secretKey;
    KGAPI2::AccountPtr account;

  private:
    Blogger * const q_ptr;
    Q_DECLARE_PUBLIC(Blogger)
};

} // namespace KBlog

using namespace KBlog;

BloggerPrivate::BloggerPrivate(Blogger *parent)
    : BlogPrivate()
    , q_ptr(parent)
{
}

BloggerPrivate::~BloggerPrivate()
{
}

void BloggerPrivate::updateKBlogPost(BlogPost *kblog, const KGAPI2::Blogger::PostPtr &postPtr)
{
    kblog->setPostId(postPtr->id());
    kblog->setTitle(postPtr->title());
    kblog->setContent(postPtr->content());
    kblog->setTags(postPtr->labels());
    kblog->setCreationDateTime(KDateTime(postPtr->published()));
    kblog->setModificationDateTime(KDateTime(postPtr->updated()));
    kblog->setLink(postPtr->url());
    kblog->setPrivate(postPtr->status() == QLatin1String("DRAFT"));
    // TODO: Try to match more?
}

KGAPI2::Blogger::PostPtr BloggerPrivate::KBlogPostToKGAPI(const BlogPost *const kblog)
{
    Q_Q(Blogger);

    KGAPI2::Blogger::PostPtr postPtr(new KGAPI2::Blogger::Post);
    postPtr->setId(kblog->postId());
    postPtr->setBlogId(q->blogId());
    postPtr->setTitle(kblog->title());
    postPtr->setContent(kblog->content());
    postPtr->setLabels(kblog->tags());
    postPtr->setPublished(kblog->creationDateTime().dateTime());
    postPtr->setUpdated(kblog->modificationDateTime().dateTime());
    postPtr->setUrl(kblog->link());
    return postPtr;
}

BlogPost BloggerPrivate::KGAPIPostToKBlogPost(const KGAPI2::Blogger::PostPtr &postPtr)
{
    BlogPost kblog;
    updateKBlogPost(&kblog, postPtr);
    return kblog;
}

QList<BlogPost> BloggerPrivate::KGAPIPostsToKBlogPosts(const KGAPI2::ObjectsList &posts)
{
    QList<BlogPost> blogPosts;
    Q_FOREACH (const KGAPI2::ObjectPtr &obj, posts) {
        blogPosts << KGAPIPostToKBlogPost(obj.dynamicCast<KGAPI2::Blogger::Post>());
    }
    return blogPosts;
}


KBlogInfo BloggerPrivate::KGAPIBlogToKBlogBlog(const KGAPI2::Blogger::BlogPtr &blogPtr)
{
    KBlogInfo kblogInfo;
    kblogInfo[QLatin1String("id")] = blogPtr->id();
    kblogInfo[QLatin1String("title")] = blogPtr->name();
    kblogInfo[QLatin1String("url")] = blogPtr->url().toString();
    kblogInfo[QLatin1String("summay")] = blogPtr->description();
    return kblogInfo;
}

QList<KBlogInfo> BloggerPrivate::KGAPIBlogsToKBlogBlogs(const KGAPI2::ObjectsList &blogs)
{
    QList<KBlogInfo> kblogInfos;
    Q_FOREACH (const KGAPI2::ObjectPtr &obj, blogs) {
        kblogInfos << KGAPIBlogToKBlogBlog(obj.dynamicCast<KGAPI2::Blogger::Blog>());
    }
    return kblogInfos;
}

BlogComment BloggerPrivate::KGAPICommentToKBlogComment(const KGAPI2::Blogger::CommentPtr &commentPtr)
{
    BlogComment kblogComment;
    kblogComment.setCommentId(commentPtr->id());
    kblogComment.setContent(commentPtr->content());
    kblogComment.setCreationDateTime(KDateTime(commentPtr->published()));
    kblogComment.setModificationDateTime(KDateTime(commentPtr->updated()));
    kblogComment.setName(commentPtr->authorName());
    return kblogComment;
}

QList<BlogComment> BloggerPrivate::KGAPICommentsToKBlogComments(const KGAPI2::ObjectsList &comments)
{
    QList<BlogComment> kblogComments;
    Q_FOREACH (const KGAPI2::ObjectPtr &obj, comments) {
        kblogComments << KGAPICommentToKBlogComment(obj.dynamicCast<KGAPI2::Blogger::Comment>());
    }
    return kblogComments;
}

bool BloggerPrivate::handleError(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!job->error()) {
        return true;
    }

    KBlog::BlogPost *post = job->property(KBLOGPOST_PROPERTY).value<BlogPost*>();

    KBlog::Blog::ErrorType errCode = Blog::Other;
    switch (job->error()) {
      case KGAPI2::Unauthorized: {
        KGAPI2::AuthJob *authJob = new KGAPI2::AuthJob(account, apiKey, secretKey, q);
        authJob->setProperty(JOB_PROPERTY, QVariant::fromValue(job));
        q->connect(authJob, SIGNAL(finished(KGAPI2::Job*)),
                   q, SLOT(_k_onAuthenticateFinished(KGAPI2::Job*)));
        return false;
      }
      case KGAPI2::AuthCancelled:
      case KGAPI2::AuthError:
        errCode = Blog::AuthenticationError;
        break;

      case KGAPI2::BadRequest:
        errCode = Blog::XmlRpc;
        break;

      // Not found is handled in callers
      case KGAPI2::NotFound:
        return true;

      default:
        errCode = Blog::Other;
        break;
    }

    if (post) {
        Q_EMIT q->errorPost(errCode, job->errorString(), post);
    } else {
        Q_EMIT q->error(errCode, job->errorString());
    }

    job->deleteLater();
    return false;
}




Blogger::Blogger(const KUrl &server, QObject *parent)
    : Blog(server, *new BloggerPrivate(this), parent)
{
    qDebug();
}

Blogger::~Blogger()
{
    qDebug();
}


QString Blogger::interfaceName() const
{
    return QLatin1String("Blogger 3.0");
}

void Blogger::setApiKey(const QString &apiKey)
{
    Q_D(Blogger);
    d->apiKey = apiKey;
}

void Blogger::setSecretKey(const QString &secretKey)
{
    Q_D(Blogger);
    d->secretKey = secretKey;
}

void Blogger::authenticate(const QMap<QString, QString> &authData)
{
    Q_D(Blogger);

    KGAPI2::AccountPtr account;
    qDebug() << authData;
    if (!authData.isEmpty()) {
        QList<QUrl> scopes;
        scopes << KGAPI2::Account::bloggerScopeUrl();
        account = KGAPI2::AccountPtr(new KGAPI2::Account(authData[QLatin1String("account")],
                                                         authData[QLatin1String("accessToken")],
                                                         authData[QLatin1String("refreshToken")],
                                                         scopes));
        d->account = account;
    } else {
        account = KGAPI2::AccountPtr(new KGAPI2::Account);
        account->addScope(KGAPI2::Account::bloggerScopeUrl());
        KGAPI2::AuthJob *authJob = new KGAPI2::AuthJob(account, d->apiKey, d->secretKey, this);
        if (account->accessToken().isEmpty()) {
            authJob->setUsername(username());
            authJob->setPassword(password());
        }
        connect(authJob, SIGNAL(finished(KGAPI2::Job*)),
                this, SLOT(_k_onAuthenticateFinished(KGAPI2::Job*)));
    }
}

void BloggerPrivate::_k_onAuthenticateFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);
    if (!handleError(job)) {
        return;
    }

    KGAPI2::AuthJob *authJob = qobject_cast<KGAPI2::AuthJob*>(job);
    account = authJob->account();

    QMap<QString, QString> authData;
    authData[QLatin1String("account")] = account->accountName();
    authData[QLatin1String("accessToken")] = account->accessToken();
    authData[QLatin1String("refreshToken")] = account->refreshToken();

    Q_EMIT q->authenticated(authData);

    if (authJob->property(JOB_PROPERTY).isValid()) {
        KGAPI2::Job *originalJob = authJob->property(JOB_PROPERTY).value<KGAPI2::Job*>();
        if (originalJob) {
            originalJob->restart();
        }
    }
}


void Blogger::listBlogs()
{
    Q_D(Blogger);

    KGAPI2::Blogger::BlogFetchJob *fetchJob
         = new KGAPI2::Blogger::BlogFetchJob(QLatin1String("self"),
                                             KGAPI2::Blogger::BlogFetchJob::FetchByUserId,
                                             d->account,
                                             this);
    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onListBlogsFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onListBlogsFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::FetchJob *fetchJob = qobject_cast<KGAPI2::FetchJob*>(job);
    const QList<KBlogInfo> blogs = KGAPIBlogsToKBlogBlogs(fetchJob->items());
    Q_EMIT q->listedBlogs(blogs);
}


void Blogger::listRecentPosts(int number)
{
    Q_D(Blogger);

    KGAPI2::Blogger::PostFetchJob *fetchJob
        = new KGAPI2::Blogger::PostFetchJob(blogId(), d->account, this);
    fetchJob->setMaxResults(number);
    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onListRecentPostsFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onListRecentPostsFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::FetchJob *fetchJob = qobject_cast<KGAPI2::FetchJob*>(job);
    QList<BlogPost> posts = KGAPIPostsToKBlogPosts(fetchJob->items());
    QList<BlogPost>::Iterator iter, endIter = posts.end();
    for (iter = posts.begin(); iter != endIter; ++iter) {
        (*iter).setStatus(BlogPost::Fetched);
    }
    Q_EMIT q->listedRecentPosts(posts);
}


void Blogger::fetchPost(KBlog::BlogPost *post)
{
    Q_D(Blogger);

    KGAPI2::Blogger::PostFetchJob *fetchJob
        = new KGAPI2::Blogger::PostFetchJob(blogId(), post->postId(), d->account, this);
    fetchJob->setProperty(KBLOGPOST_PROPERTY, QVariant::fromValue(post));
    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onFetchPostFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onFetchPostFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::FetchJob *fetchJob = qobject_cast<KGAPI2::FetchJob*>(job);
    BlogPost *kblog = fetchJob->property(KBLOGPOST_PROPERTY).value<BlogPost*>();
    const KGAPI2::ObjectsList items = fetchJob->items();
    if (items.count() != 1) {
        Q_EMIT q->errorPost(Blog::Other, i18n("Blog post not found"), kblog);
        return;
    }

    updateKBlogPost(kblog, items.first().dynamicCast<KGAPI2::Blogger::Post>());
    kblog->setStatus(BlogPost::Fetched);
    Q_EMIT q->fetchedPost(kblog);
}


void Blogger::removePost(KBlog::BlogPost *post)
{
    Q_D(Blogger);

    KGAPI2::Blogger::PostDeleteJob *deleteJob
        = new KGAPI2::Blogger::PostDeleteJob(blogId(), post->postId(), d->account, this);
    deleteJob->setProperty(KBLOGPOST_PROPERTY, QVariant::fromValue(post));
    connect(deleteJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onRemovePostFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onRemovePostFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    BlogPost *kblog = job->property(KBLOGPOST_PROPERTY).value<BlogPost*>();
    kblog->setStatus(BlogPost::Removed);
    Q_EMIT q->removedPost(kblog);
}



void Blogger::createPost(KBlog::BlogPost *post)
{
    Q_D(Blogger);

    KGAPI2::Blogger::PostPtr postPtr = d->KBlogPostToKGAPI(post);
    KGAPI2::Blogger::PostCreateJob *createJob
        = new KGAPI2::Blogger::PostCreateJob(postPtr, post->isPrivate(), d->account, this);
    createJob->setProperty(KBLOGPOST_PROPERTY, QVariant::fromValue(post));
    connect(createJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onCreatePostFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onCreatePostFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::CreateJob *createJob = qobject_cast<KGAPI2::CreateJob*>(job);
    BlogPost* kblog = createJob->property(KBLOGPOST_PROPERTY).value<BlogPost*>();
    const KGAPI2::ObjectsList items = createJob->items();
    if (items.count() != 1) {
        Q_EMIT q->errorPost(Blog::Other, i18n("Failed to create new post"), kblog);
        return;
    }

    updateKBlogPost(kblog, items.first().dynamicCast<KGAPI2::Blogger::Post>());
    kblog->setStatus(BlogPost::Created);
    Q_EMIT q->createdPost(kblog);
}


void Blogger::modifyPost(KBlog::BlogPost *post)
{
    Q_D(Blogger);

    KGAPI2::Blogger::PostPtr postPtr = d->KBlogPostToKGAPI(post);
    postPtr->setPublished(QDateTime());
    postPtr->setUpdated(QDateTime());
    KGAPI2::Blogger::PostModifyJob *modifyJob
        = new KGAPI2::Blogger::PostModifyJob(postPtr, d->account, this);
    modifyJob->setProperty(KBLOGPOST_PROPERTY, QVariant::fromValue(post));
    connect(modifyJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onModifyPostFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onModifyPostFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::ModifyJob *modifyJob = qobject_cast<KGAPI2::ModifyJob*>(job);
    BlogPost *kblog = modifyJob->property(KBLOGPOST_PROPERTY).value<BlogPost*>();
    const KGAPI2::ObjectsList items = modifyJob->items();
    if (items.count() != 1) {
        Q_EMIT q->errorPost(Blog::Other, i18n("Failed to update post"), kblog);
        return;
    }

    updateKBlogPost(kblog, items.first().dynamicCast<KGAPI2::Blogger::Post>());
    kblog->setStatus(BlogPost::Modified);
    Q_EMIT q->modifiedPost(kblog);
}


void Blogger::listComments(BlogPost *post)
{
    Q_D(Blogger);

    KGAPI2::Blogger::CommentFetchJob *fetchJob
         = new KGAPI2::Blogger::CommentFetchJob(blogId(), post->postId(), d->account, this);
    fetchJob->setProperty(KBLOGPOST_PROPERTY, QVariant::fromValue(post));
    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(_k_onListCommentsFinished(KGAPI2::Job*)));
}

void BloggerPrivate::_k_onListCommentsFinished(KGAPI2::Job *job)
{
    Q_Q(Blogger);

    if (!handleError(job)) {
        return;
    }

    job->deleteLater();
    KGAPI2::FetchJob *fetchJob = qobject_cast<KGAPI2::FetchJob*>(job);
    BlogPost *kblog = fetchJob->property(KBLOGPOST_PROPERTY).value<BlogPost*>();
    const QList<KBlog::BlogComment> comments = KGAPICommentsToKBlogComments(fetchJob->items());
    Q_EMIT q->listedComments(kblog, comments);
}



#include "moc_blogger.cpp"

