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

#ifndef ADDEDITBLOG_H
#define ADDEDITBLOG_H

#include <KDialog>

#include "ui_addeditblogbase.h"
#include "bilboblog.h"

#include <KBlog/blog.h>

class BilboBlog;
class KJob;
class WaitWidget;

/**
 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
 */
class AddEditBlog: public KDialog
{
    Q_OBJECT
public:
    /**
     *
     * @param blog_id id of blog we will edit it, for adding a blog this should be "-1"
     * @param parent Parent
     */
    explicit AddEditBlog( int blog_id, QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~AddEditBlog();

protected Q_SLOTS:
    virtual void slotButtonClicked( int button );
    void enableAutoConfBtn();
    void enableOkButton( const QString & );
    void autoConfigure();
    void fetchBlogId();

    void fetchedBlogId( const QList<QMap<QString, QString> >& list );
    void fetchedProfileId( const QString& );

    void handleFetchIDTimeout();
    void handleFetchAPITimeout();
    void handleFetchError( KBlog::Blog::ErrorType type, const QString& errorMsg );

    void slotReturnPressed();
    void setSupportedFeatures( BilboBlog::ApiType api );
    void slotComboApiChanged( int index );

    void gotHtml( KJob * );
    void gotXmlRpcTest( KJob *job );
Q_SIGNALS:
    void sigBlogAdded( const BilboBlog& );
    void sigBlogEdited( const BilboBlog& );

private:
    void showWaitWidget( QString text );
    void hideWaitWidget();

    class Private;
    Private * const d;
};

#endif
