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

#ifndef BILBOPOST_H
#define BILBOPOST_H

#include "category.h"

#include <kblog/blogpost.h>


class BilboPostPrivate;

/**
Definition of a blog post!
it's implemented to decrease dependency to KBlog :)
 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class BilboPost : public KBlog::BlogPost
{
public:
    BilboPost();
    explicit BilboPost( const KBlog::BlogPost& );
    BilboPost( const BilboPost& );
    ~BilboPost();

    QString author() const;
    void setAuthor( const QString& );

    int id() const;
    void setId( const int );

    //To be used for local_post and temp_post tabels
    int localId() const;
    void setLocalId( const int );

    QString toString() const;

    bool isModifyTimeStamp() const;
    void setModifyTimeStamp( bool willModify );

    QList<Category> categoryList() const;
    void setCategoryList( const QList<Category> &list );

    /**
     * Set all properties of post to new one, instead of Title and Content!
    */
    void setProperties( const BilboPost& postProp );

    /**
      The overloaed = operator.
    */
    BilboPost &operator=( const BilboPost &other );

private:
    BilboPostPrivate * d_ptr;
};

#endif
