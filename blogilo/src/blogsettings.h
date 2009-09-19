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

#ifndef BLOGSETTINGS_H
#define BLOGSETTINGS_H

#include <QWidget>
#include "bilboblog.h"
#include "ui_blogsettingsbase.h"

class BlogSettings : public QWidget, public Ui::SettingsBlogsBase
{
    Q_OBJECT
public:
    BlogSettings( QWidget *parent = 0 );
    ~BlogSettings();

signals:
    void blogAdded( const BilboBlog &blog );
    void blogEdited( const BilboBlog &blog );
    void blogRemoved( int blog_id );

protected slots:
    void addBlog();
    void editBlog();
    void removeBlog();
    void loadBlogsList();

private slots:
    void slotBlogAdded( const BilboBlog &blog );
    void slotBlogEdited( const BilboBlog &blog );
    void blogsTablestateChanged();

private:
    void addBlogToList( const BilboBlog &blog );
};

#endif // SETTINGSBLOGS_H
