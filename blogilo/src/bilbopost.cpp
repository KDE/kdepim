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
#include <kdatetime.h>
#include <qdebug.h>
#include <QStringList>

class BilboPostPrivate
{
public:
    QString mAuthor;
    int mId;///id in DB
    int localId;
    bool mModifyTimeStamp;///Just for toolbox entry!
    QList<Category> mCategoryList;
};

BilboPost::BilboPost()
    : KBlog::BlogPost(), d_ptr(new BilboPostPrivate)
{
    this->setCreationDateTime(KDateTime::currentLocalDateTime());
    this->setModificationDateTime(KDateTime::currentLocalDateTime());
    this->setCommentAllowed(true);
    this->setPrivate(false);
    this->setTrackBackAllowed(true);
    d_ptr->mModifyTimeStamp = false;
    this->setId(-1);
    this->setLocalId(-1);
    this->setStatus(KBlog::BlogPost::New);
}

BilboPost::BilboPost(const KBlog::BlogPost &post)
    : KBlog::BlogPost(post), d_ptr(new BilboPostPrivate)
{
    d_ptr->mId = -1;
    d_ptr->localId = -1;
    d_ptr->mModifyTimeStamp = false;
}

BilboPost::BilboPost(const BilboPost &post)
    : KBlog::BlogPost(post), d_ptr(new BilboPostPrivate)
{
    this->setAuthor(post.author());
    this->setModifyTimeStamp(post.isModifyTimeStamp());
    this->setId(post.id());
    setLocalId(post.localId());
    this->setCategoryList(post.categoryList());
}

BilboPost::~BilboPost()
{
    delete d_ptr;
}

QString BilboPost::author() const
{
    return d_ptr->mAuthor;
}

void BilboPost::setId(const int id)
{
    d_ptr->mId = id;
}

int BilboPost::id() const
{
    return d_ptr->mId;
}

void BilboPost::setAuthor(const QString &author)
{
    d_ptr->mAuthor = author;
}

int BilboPost::localId() const
{
    return d_ptr->localId;
}

void BilboPost::setLocalId(const int localId)
{
    d_ptr->localId = localId;
}

QString BilboPost::toString() const
{
    //  if(!title().isEmpty())
    //   qDebug("BilboPost::toString: title is %s", this->title());
    QString ret;
    ret = QLatin1String("\n******* Post Info **********");
    ret += QLatin1String("\nID: ") + postId();
    ret += QLatin1String("\nTitle: ") + title();
    ret += QLatin1String("\nContent: ") + content();
    ret += QLatin1String("\nAdditionalContent: ") + additionalContent();
    ret += QLatin1String("\nTags: ") + tags().join(QLatin1String(","));
    ret += QLatin1String("\nCategories: ") + categories().join(QLatin1String(","));
    ret += QLatin1String("\nCreation Date Time: ") + creationDateTime().toString();
    ret += QLatin1String("\nStatus: ") + QString::number(status());
    ret += QLatin1String("\nIsPrivate: ") + QVariant(isPrivate()).toString();
    ret += QLatin1String("\n******* End Post Info ********\n");
    return ret;
}

bool BilboPost::isModifyTimeStamp() const
{
    return d_ptr->mModifyTimeStamp;
}

void BilboPost::setModifyTimeStamp(bool isModify)
{
    d_ptr->mModifyTimeStamp = isModify;
}

QList< Category > BilboPost::categoryList() const
{
    return d_ptr->mCategoryList;
}

void BilboPost::setCategoryList(const QList< Category > &list)
{
    d_ptr->mCategoryList = list;
    QStringList cats;
    const int count = list.count();
    for (int i = 0; i < count; ++i) {
        cats.append(list.at(i).name);
    }
    setCategories(cats);
}

void BilboPost::setProperties(const BilboPost &postProp)
{
    qDebug();
    this->setCreationDateTime(postProp.creationDateTime());
    this->setModificationDateTime(postProp.modificationDateTime());
    this->setLink(postProp.link());
    this->setPermaLink(postProp.permaLink());
    this->setCategories(postProp.categories());
    this->setCategoryList(postProp.categoryList());
    this->setCommentAllowed(postProp.isCommentAllowed());
    this->setTags(postProp.tags());
    this->setMood(postProp.mood());
    this->setMusic(postProp.music());
    this->setSummary(postProp.summary());
    this->setTrackBackAllowed(postProp.isTrackBackAllowed());
    this->setAuthor(postProp.author());
    this->setModifyTimeStamp(postProp.isModifyTimeStamp());
    this->setSlug(postProp.slug());
}

BilboPost &BilboPost::operator=(const BilboPost &other)
{
    KBlog::BlogPost other2 = KBlog::BlogPost::operator=(other);
    BilboPost copy(other2);
    swap(copy);
    return *this;
}
