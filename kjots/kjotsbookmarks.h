//
//  kjotsbookmarks
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef KJOTSBOOKMARKS
#define KJOTSBOOKMARKS

#include <kbookmarkmanager.h>

class KJotsTreeView;

class KJotsBookmarks : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    explicit KJotsBookmarks( KJotsTreeView *treeView );
    ~KJotsBookmarks();

    virtual QUrl currentUrl() const;
    virtual QString currentTitle() const;
    virtual void openBookmark( const KBookmark & bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km );

private:
    KJotsTreeView *m_treeView;
};

#endif
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
