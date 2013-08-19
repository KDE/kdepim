/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013 Laurent Montel <montel@kde.org> 

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

#ifndef BILBOBROWSER_H
#define BILBOBROWSER_H

#include <QWidget>

#include "kurl.h"

class QCheckBox;
class QProgressBar;
class KWebView;
class KPushButton;
class KStatusBar;

/**
* Implements a simple browser widget for use in blogilo Post Preview.
* In addition to normal browsing tools, it has a button for fetching blog styles
* from the web.
This class will use on Win32!
    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
    @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class BilboBrowser: public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief BilboBrowser constructor.
     * Creates a widget that consists of a KHTMLView, some common browsing 
     * tools, and a button for getting blog style.
     * @param parent is parent widget
     */
    explicit BilboBrowser( QWidget *parent = 0 );

    /**
     * @brief BilboBrowser destructor.
     */
    ~BilboBrowser();

    /**
     * Shows a post with given title and content in the browser.
     * @param title is the post title.
     * @param content is the post content.
     */
    void setHtml( const QString& title, const QString& content );

    void stop();

Q_SIGNALS:
    /**
     * This signal is emmited when the browser finishes getting blog style.
     */
    void sigSetBlogStyle();

protected Q_SLOTS:
    void slotGetBlogStyle();

    void slotSetBlogStyle();

    void slotCompleted( bool );

    void slotSetStatusBarText( const QString& text );

    void slotViewModeChanged();

private:
    void createUi( QWidget *parent );

    KWebView *mWebView;
    QCheckBox *viewInBlogStyle;
    KPushButton *btnGetStyle;
    QProgressBar *browserProgress;
    KStatusBar *browserStatus;

    QString currentTitle;
    QString currentContent;
};

#endif
