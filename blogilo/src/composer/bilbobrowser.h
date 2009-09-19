/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef BILBOBROWSER_H
#define BILBOBROWSER_H

#include <QWidget>

#include "kurl.h"
// #include "kparts/part.h"
// #include "kparts/browserextension.h"

class QCheckBox;
class QProgressBar;
class KHTMLPart;
class KPushButton;
class KStatusBar;

/**
* Implements a simple browser widget for use in bilbo Post Preview.
* In addition to normal browsing tools, it has a button for fetching blog styles
* from the web.
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
    BilboBrowser( QWidget *parent = 0 );

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
/*
    void setBrowserDirection( Qt::LayoutDirection direction );*/

Q_SIGNALS:
    /**
     * This signal is emmited when the browser finishes getting blog style.
     */
    void sigSetBlogStyle();

protected Q_SLOTS:
    void sltGetBlogStyle();

    void sltSetBlogStyle();

    void sltCompleted();

    void sltCanceled( const QString& errMsg );

    void sltSetStatusBarText( const QString& text );

    void sltViewModeChanged();

    void sltOpenRequested( const KUrl& url );

private:
    void createUi( QWidget *parent = 0 );

    KHTMLPart *browserPart;

    QCheckBox *viewInBlogStyle;
    KPushButton *btnGetStyle;
    QProgressBar *browserProgress;
    KStatusBar *browserStatus;

    QString currentTitle;
    QString currentContent;
};

#endif
