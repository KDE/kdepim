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

#ifndef POSTENTRY_H
#define POSTENTRY_H

#include <QFrame>
#include "bilbopost.h"

class BilboMedia;
class Backend;

/**
  Post Entry Widget
  contains Editor, and Title box.
 @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 @author Golnaz Nilieh \<g382nilieh@gmail.com\>
 */
class PostEntry: public QFrame
{
    Q_OBJECT
public:
    explicit PostEntry(QWidget *parent);
    ~PostEntry();
    QString postTitle() const;

    void setPostTitle(const QString &title);
    void setPostBody(const QString &content, const QString &additionalContent = QString());

    int currentPostBlogId() const;
    void setCurrentPostBlogId(int blog_id);

    BilboPost *currentPost();
    void setCurrentPost(const BilboPost &post);

    Qt::LayoutDirection defaultLayoutDirection() const;
    void setDefaultLayoutDirection(Qt::LayoutDirection direction);

    /**
     * Will Upload media files not uploaded yet, and return true on success and false on failure.
     * @param backend A Backend instant to use! will create one if NULL
     * @return true on success and false on failure.
     */
    bool uploadMediaFiles(Backend *backend = 0);

    void submitPost(int blogId, const BilboPost &postData);

    void saveLocally();

Q_SIGNALS:
    /**
     * emitted when title of this entry changed.
     * @param title is a QString which contains new title.
     */
    void postTitleChanged(const QString &title);
    /**
     * This signal emitted when a post manipulation job e.g. Publishing a new post finished.
     * @param isError If an error occurred on publishing this will be TRUE. Otherwise FLASE
     * @param customMessage A Custom message will be shown on StatusBar.
     */
    void postPublishingDone(bool isError, const QString &customMessage);

    /**
     * This signal is emitted when the content of VisualEditor or HtmlEditor changes.
     */
    void textChanged();

    /**
     * This signal is emitted when the post is saved temporarily!
     */
    void postSavedTemporary();

    void postSavedLocally();

    /**
     * To show a message on statusBar
     * @param message Message to be shown
     * @param isPermanent If it's true the message will not have a timeout!
     *  so it will be shown until next message arrived
     */
    void showStatusMessage(const QString &message, bool isPermanent);

    /**
     * This signal is emitted for operations in background, like request of some
     * data from the web.
     * @param isBusy if it's true, the operation is in progress. otherwise, it
     * is finished.
     */
    void sigBusy(bool isBusy);

public Q_SLOTS:
    void settingsChanged();

protected Q_SLOTS:
    void slotError(const QString &errMsg);
    void slotPostPublished(int blog_id, BilboPost *post);
    void slotTitleChanged();
    void showProgressBar();
    void deleteProgressBar();
    void saveTemporary();
    void slotPostModified();
    void slotFocusEditor();

    /*!
     *  Sets the content of the current tab  as other tabs' contents, to apply recent
     * changes. this function executes each time the user switches between tabs.
     */
    void slotSyncEditors(int index);

    void slotSetPostPreview();

protected:
    /**
     * @brief Returns the editor current text in html format
     * Synchronizes HtmlEditor and editor tabs, by sending content of the current one to another.
     * then copies the content of HtmlEditor into the variable mHtmlContent, and returns it.
     * @return an String which contains html text
     */
    QString htmlContent() const;

    QString plainTextContent() const;

    /**
     * Sets the given string as the HtmlEditor and VisualEditor content.
     * @param content
     */
    void setHtmlContent(const QString &content);

    QList <BilboMedia *> localImages() const;
    void replaceImageSrc(const QString &src, const QString &dest);

private:
    void createUi();
    void setCurrentPostFromEditor();

    class Private;
    Private *const d;
};

#endif
