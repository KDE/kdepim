/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SIEVEEDITORBOOKMARKS_H
#define SIEVEEDITORBOOKMARKS_H

#include <QObject>
#include <kbookmarkowner.h>
class KBookmarkMenu;
class QMenu;
class KActionCollection;
class SieveEditorMainWindow;
class SieveEditorBookmarks : public QObject, public KBookmarkOwner
{
    Q_OBJECT
public:
    explicit SieveEditorBookmarks(SieveEditorMainWindow *mainWindow, KActionCollection *collection, QMenu *menu, QObject *parent = Q_NULLPTR);
    ~SieveEditorBookmarks();

    QMenu *menu() const;

    void openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) Q_DECL_OVERRIDE;

    QString currentTitle() const Q_DECL_OVERRIDE;
    QUrl currentUrl() const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void openUrl(const QUrl &url);

private:
    KBookmarkMenu *mBookmarkMenu;
    QMenu *mMenu;
    SieveEditorMainWindow *mMainWindow;
};

#endif // SIEVEEDITORBOOKMARKS_H
