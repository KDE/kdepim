/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "bilbopost.h"
#include <QStringList>
#include <kdatetime.h>
#include <kdebug.h>

BilboPost::BilboPost()
        : KBlog::BlogPost()
{
    kDebug();
    this->setCreationDateTime( KDateTime::currentLocalDateTime() );
    this->setModificationDateTime( KDateTime::currentLocalDateTime() );
//     this->setLink( KUrl() );
//     this->setPermaLink( KUrl() );
//     this->setCategories( QStringList() );
//     this->setCategoryList( QList<Category>() );
    this->setCommentAllowed( true );
//     this->setContent( QString() );
//     this->setTags( QStringList() );
//     this->setMood( QString() );
//     this->setMusic( QString() );
    this->setPrivate( false );
//     this->setSummary( QString() );
    this->setTrackBackAllowed( true );
//     this->setTitle( QString() );
//     this->setAuthor( QString() );
    this->mModifyTimeStamp = false;
    this->setId( -1 );
    this->setStatus( KBlog::BlogPost::New );
}

BilboPost::~BilboPost()
{
    kDebug();
}

QString BilboPost::author() const
{
    return this->mAuthor;
}

void BilboPost::setId( const int id )
{
    this->mId = id;
}

int BilboPost::id() const
{
    return this->mId;
}

void BilboPost::setAuthor( const QString &author )
{
    this->mAuthor = author;
}

BilboPost::BilboPost( const KBlog::BlogPost &post )
        : KBlog::BlogPost( post )
{
    mId = -1;
    this->mModifyTimeStamp = false;
}

KBlog::BlogPost * BilboPost::toKBlogPost()
{
    KBlog::BlogPost *pp = new KBlog::BlogPost( QString() );
    pp->setStatus( this->status() );
    pp->setSummary( this->summary() );
    pp->setTags( this->tags() );
    pp->setTitle( this->title() );
    pp->setTrackBackAllowed( this->isTrackBackAllowed() );
    pp->setCategories( this->categories() );
    pp->setCommentAllowed( this->isCommentAllowed() );
    pp->setContent( this->content() );
    pp->setPrivate( this->isPrivate() );
    pp->setCreationDateTime( this->creationDateTime() );
    pp->setError( this->error() );
    pp->setModificationDateTime( this->modificationDateTime() );
    pp->setMood( this->mood() );
    pp->setMusic( this->music() );
    pp->setPostId( this->postId() );
    pp->setLink( this->link() );
    pp->setPermaLink( this->permaLink() );
    pp->setStatus( this->status() );
    pp->setAdditionalContent( this->additionalContent() );
    pp->setSlug( this->slug() );

    return pp;
}

QString BilboPost::toString() const
{
    qDebug( "BilboPost::toString" );
//  if(!title().isEmpty())
//   qDebug("BilboPost::toString: title is %s", this->title());
    QString ret;
    ret = "\n******* Post Info **********";
    ret += QString( "\nID: " ) + postId();
    ret += QString( "\nTitle: " ) + title();
    ret += QString( "\nContent: " ) + content();
    ret += "\nAdditionalContent: " + additionalContent();
    ret += "\nTags: " + tags().join( "," );
    ret += "\nCategories: " + categories().join( "," );
    ret += "\nCreation Date Time: " + creationDateTime().toString();
    ret += "\nStatus: " + QString::number(status());
    ret += "\nIsPrivate: " + QVariant(isPrivate()).toString();
    ret += "\n******* End Post Info ********\n";
    return ret;
}

BilboPost::BilboPost( const BilboPost &post )
        : KBlog::BlogPost()
{
    kDebug();
    this->setPostId(post.postId());
    this->setCreationDateTime( post.creationDateTime() );
    this->setModificationDateTime( post.modificationDateTime() );
    this->setLink( post.link() );
    this->setPermaLink( post.permaLink() );
    this->setCategories( post.categories() );
    this->setCategoryList( post.categoryList() );
    this->setCommentAllowed( post.isCommentAllowed() );
    this->setError( post.error() );
    this->setContent( post.content() );
    this->setTags( post.tags() );
    this->setMood( post.mood() );
    this->setMusic( post.music() );
    this->setPrivate( post.isPrivate() );
    this->setStatus( post.status() );
    this->setSummary( post.summary() );
    this->setTrackBackAllowed( post.isTrackBackAllowed() );
    this->setTitle( post.title() );
    this->setAuthor( post.author() );
    this->setModifyTimeStamp( post.isModifyTimeStamp() );
    this->setId( post.id() );
    this->setStatus( post.status() );
    this->setAdditionalContent( post.additionalContent() );
    this->setSlug( post.slug() );
}

bool BilboPost::isModifyTimeStamp() const
{
    return mModifyTimeStamp;
}

void BilboPost::setModifyTimeStamp( bool isModify )
{
    mModifyTimeStamp = isModify;
}

QList< Category > BilboPost::categoryList() const
{
    return mCategoryList;
}

void BilboPost::setCategoryList( const QList< Category > & list )
{
    mCategoryList = list;
    QStringList cats;
    int count = list.count();
    for ( int i = 0; i < count; ++i ) {
        cats.append( list[i].name );
    }
    this->setCategories( cats );
}

void BilboPost::setProperties( const BilboPost& postProp )
{
    kDebug();
    this->setCreationDateTime( postProp.creationDateTime() );
    this->setModificationDateTime( postProp.modificationDateTime() );
    this->setLink( postProp.link() );
    this->setPermaLink( postProp.permaLink() );
    this->setCategories( postProp.categories() );
    this->setCategoryList( postProp.categoryList() );
    this->setCommentAllowed( postProp.isCommentAllowed() );
    this->setTags( postProp.tags() );
    this->setMood( postProp.mood() );
    this->setMusic( postProp.music() );
    this->setSummary( postProp.summary() );
    this->setTrackBackAllowed( postProp.isTrackBackAllowed() );
    this->setAuthor( postProp.author() );
    this->setModifyTimeStamp( postProp.isModifyTimeStamp() );
    this->setSlug( postProp.slug() );
}
