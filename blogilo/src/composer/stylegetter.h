/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "weblogstylegetter.h" from
    KBlogger project. it has been modified for use in Blogilo, at
    February 2009.

    Copyright (C) 2007-2008-2008 by Christian Weilbach <christian_weilbach@web.de>
    Copyright (C) 2007-2008 Antonio Aloisio <gnuton@gnuton.org>
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

#ifndef STYLEGETTER_H
#define STYLEGETTER_H
#include <QObject>
#include <QString>

class KJob;

class BilboPost;
class Backend;

/**
  @brief
  This class will:
  1. Create a temporary post
  2. Send post
  3. Read the post's url
  4. Get & save the html page
*/
class StyleGetter: public QObject
{
    Q_OBJECT
public:
    /**
     * Creates an instance of StyleGetter, to fetch the style of the requested
     * blog.
     * @param blogid requested blog's id, to fetch style.
     * @param parent
     */
    StyleGetter(const int blogid, QObject *parent);
    /**
     * StyleGetter destructor.
     */
    ~StyleGetter();

    /**
     * Looks for a file named style.html for the requested blog in blogilo data
     * directory, then writes the file content into a buffer, inserts given post
     * title and content in the buffer, and returns it.
     * @param blogid the id of the requested blog
     * @param title title of a post to be styled.
     * @param content content of that post to be styled.
     * @return an html string which shows the post in the blog template.
     */
    static QString styledHtml(const int blogid,
                              const QString &title,
                              const QString &content);
Q_SIGNALS:
    /**
     * When StyleGetter finishes all jobs to get and save a blog style, this
     * signal will be emmited.
     */
    void sigStyleFetched();

    /**
     * While the class is fetching blog style from the web, this signal shows
     * the operation progress.
     * @param percent is a number between 0 to 100, showing the progress in percent
     */
    void sigGetStyleProgress(int percent);

private Q_SLOTS:
//     void slotPostSent();
    void slotTempPostPublished(int blogId, BilboPost *post);
    void slotTempPostRemoved(int blog_id, const BilboPost &post);
    void slotHtmlCopied(KJob *job);
    void slotError(const QString &errMsg);

private:
    void generateRandomPostStrings();

    BilboPost *mTempPost;
    QString mCachePath;
    QString mPostTitle;
    QString mPostContent;
    QWidget *mParent;

    Backend *b;
};

#endif
