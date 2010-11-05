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

#ifndef BILBOEDITOR_H
#define BILBOEDITOR_H

#include <QTextFormat>

#include <ktabwidget.h>
#include <KUrl>

class TextEditor;
class QTextCharFormat;
// class QProgressBar;
// class QWebView;
// class QPlainTextEdit;
namespace KTextEditor
{ 
    class View;
}

class KAction;
class KToolBar;
class KListWidget;
// class KPushButton;
class KSelectAction;
// class KStatusBar;
// class MultiLineTextEdit;
class AddEditLink;
class BilboMedia;
class BilboBrowser;


/**
* Class BilboEditor represents the editor part of BilboBlogger
* @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
* @author Golnaz Nilieh <g382nilieh@gmail.com>
*/

class BilboEditor : public KTabWidget
{
    Q_OBJECT
public:

    /**
     * @brief BilboEditor constructor.
     * Creates a new instance of BilboEditor, and calls createUi() function to 
     * initialize its widgets.
     */
    BilboEditor( QWidget *parent = 0 );

    /**
     * @brief BilboEditor destructor.
     */
    ~BilboEditor();

    /**
     * @brief Returns the editor current text in html format
     * Synchronizes HtmlEditor and editor tabs, by sending content of the current one to another.
     * then copies the content of HtmlEditor into the variable mHtmlContent, and returns it.
     * @return an String which contains html text
     */
    QString htmlContent();

    QString plainTextContent();

    /**
     * Sets the given string as the HtmlEditor and VisualEditor content.
     * @param content
     */
    void setHtmlContent( const QString &content );

    QList <BilboMedia*> localImages();

    /**
     * Stores the given title, to be shown as the post title in preview tab.
     * @param title is the post title.
     */
    void setCurrentTitle( const QString& title );

    void replaceImageSrc(const QString& src, const QString& dest);

Q_SIGNALS:
    /**
     * This signal is emitted when the content of VisualEditor or HtmlEditor changes.
     */
    void textChanged();

    /**
     * To show a message on statusBar
     * @param message Message to be shown
     * @param isPermanent If it's true the message will not have a timeout!
     *  so it will be shown until next message arrived
     */
    void sigShowStatusMessage( const QString& message, bool isPermanent );

    /**
     * This signal is emitted for operations in background, like request of some
     * data from the web.
     * @param isBusy if it's true, the operation is in progress. otherwise, it
     * is finished.
     */
    void sigBusy( bool isBusy );

protected Q_SLOTS:
    void slotSettingsChanged();

//     void slotToggleSpellChecking();

//     void slotSyncSpellCheckingButton( bool check );

    /**
     * Creates an instance of AddImageDialog class,and opens it, to select an image.
     */
    void slotAddImage();

    void slotSetImage( BilboMedia *media, const int width, const int height, 
               const QString title, const QString link, const QString Alt_text );

    /*!
    Sets the content of the current tab  as other tabs' contents, to apply recent
     * changes. this function executes each time the user switches between tabs.
     */
    void slotSyncEditors( int index );

//     void slotGetBlogStyle();
// 
    void slotSetPostPreview();

private:

    /*!
    Creates Widget of BilboEditor.
    then assigns default charachter format of the editor tab to defaultCharFormat
     * variable, to be used in remove formatting operation. then calls createActions
     * function.
    \sa slotRemoveFormatting(), createActions()
     */
    void createUi();

    class Private;
    Private * const d;
};

#endif
