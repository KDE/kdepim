/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

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

class BilboPostPrivate
{
public:
    QString mAuthor;
    int mId;///id in DB
//  Position mPosition;
    bool mModifyTimeStamp;///Just for toolbox entry!
    QList<Category> mCategoryList;
};

BilboPost::BilboPost()
        : KBlog::BlogPost(), d_ptr(new BilboPostPrivate)
{
    kDebug();
    this->setCreationDateTime( KDateTime::currentLocalDateTime() );
    this->setModificationDateTime( KDateTime::currentLocalDateTime() );
    this->setCommentAllowed( true );
    this->setPrivate( false );
    this->setTrackBackAllowed( true );
    d_ptr->mModifyTimeStamp = false;
    this->setId( -1 );
    this->setStatus( KBlog::BlogPost::New );
}

BilboPost::BilboPost( const KBlog::BlogPost &post )
        : KBlog::BlogPost( post ), d_ptr(new BilboPostPrivate)
{
    kDebug()<<"KBlog::BlogPost";
    d_ptr->mId = -1;
    d_ptr->mModifyTimeStamp = false;
}

BilboPost::BilboPost( const BilboPost &post )
        : KBlog::BlogPost(post), d_ptr(new BilboPostPrivate)
{
    kDebug()<<"BilboPost";
    this->setAuthor( post.author() );
    this->setModifyTimeStamp( post.isModifyTimeStamp() );
    this->setId( post.id() );
    this->setCategoryList( post.categoryList() );
}

BilboPost::~BilboPost()
{
    kDebug();
    delete d_ptr;
}

QString BilboPost::author() const
{
    return d_ptr->mAuthor;
}

void BilboPost::setId( const int id )
{
    d_ptr->mId = id;
}

int BilboPost::id() const
{
    return d_ptr->mId;
}

void BilboPost::setAuthor( const QString &author )
{
    d_ptr->mAuthor = author;
}
/*
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
}*/

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

bool BilboPost::isModifyTimeStamp() const
{
    return d_ptr->mModifyTimeStamp;
}

void BilboPost::setModifyTimeStamp( bool isModify )
{
    d_ptr->mModifyTimeStamp = isModify;
}

QList< Category > BilboPost::categoryList() const
{
    return d_ptr->mCategoryList;
}

void BilboPost::setCategoryList( const QList< Category > & list )
{
    d_ptr->mCategoryList = list;
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

BilboPost& BilboPost::operator=(const BilboPost& other)
{
    KBlog::BlogPost other2 = KBlog::BlogPost::operator=(other);
    BilboPost copy( other2 );
    swap( copy );
    return *this;
}
