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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>
#include "bilbopost.h"
#include "bilboblog.h"
#include <QPointer>

class KStatusNotifierItem;
class KTabWidget;
class QProgressBar;
class QToolButton;
class Toolbox;
class KSelectAction;
class PostEntry;

/**
Main window of blogilo...

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    MainWindow();

    ~MainWindow();

//     static KTextEditor::editor& htmlEditor();

signals:
    void mediaFilesUploaded( int count );
    void settingsChanged();

protected slots:
    void slotOpenCurrentBlogInBrowser();
    void slotSettingsChanged();
    void slotCreateNewPost();
    void addBlog();
    void slotUploadAllChanges();
    void slotPostTitleChanged( const QString& title );
    void slotToggleToolboxVisible( bool isVisible );
    void slotToolboxVisibilityChanged( bool isVisible );
    void slotActivePostChanged( int tabIndex );
    void slotNewPostOpened( BilboPost &newPost, int blog_id );

    void slotPublishPost();

    /**
     * Remove @p widget from main tab wigdet (if it's NULL current Post entry will be removed)
     */
    void slotRemovePostEntry( PostEntry *widget=0 );

    void slotSavePostLocally();
//     void slotSaveAsDraft();

    void slotError( const QString &errorMessage );

    /**
    To open temporary posts and that posts are open at previous quit.
    */
    void loadTempPosts();
    void uploadMediaObject();

private slots:
    void optionsPreferences();
    void postManipulationDone( bool isError, const QString &customMessage );
    void slotBusy( bool isBusy );
    void slotShowStatusMessage(const QString &message, bool isPermanent);
    void currentBlogChanged( QAction* );

    void slotBlogAdded( const BilboBlog &blog );
    void slotBlogEdited( const BilboBlog &blog );
    void slotBlogRemoved( int blog_id );

    void slotDialogDestroyed( QObject *win );

protected:
    void keyPressEvent( QKeyEvent * event );
    virtual bool queryExit();

private:
    void setCurrentBlog( int blog_id );
    void setupActions();
    void setupSystemTray();
    void writeConfigs();
    /**
        Create a new post entry, 
        and return pointer to it's widget (Actually return value is a PostEntry instance)
    */
    QWidget* createPostEntry(int blog_id, const BilboPost& post);

    Toolbox *toolbox;
    QDockWidget *toolboxDock;
    QPointer<PostEntry> activePost;
    KStatusNotifierItem *systemTray;
    KTabWidget *tabPosts;

    QToolButton *btnRemovePost;
    int previousActivePostIndex;

    int busyNumber;///If this is 0 so there isn't any progress! otherwise there is! so progressbar will show
    QProgressBar *progress;

    KSelectAction *blogs;
    int &mCurrentBlogId;
};
#endif
