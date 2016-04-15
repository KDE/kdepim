/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorbookmarks.h"
#include "sieveeditormainwindow.h"
#include <QStandardPaths>
#include <kbookmarkmanager.h>
#include <KBookmarkMenu>
#include <QDir>

SieveEditorBookmarks::SieveEditorBookmarks(SieveEditorMainWindow *mainWindow, KActionCollection *collection, QMenu *menu, QObject *parent)
    : QObject(parent),
      KBookmarkOwner(),
      mBookmarkMenu(Q_NULLPTR),
      mMenu(menu),
      mMainWindow(mainWindow)
{
    QString bookmarkFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sieveeditor/bookmarks.xml"));

    if (bookmarkFile.isEmpty()) {
        bookmarkFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/sieveeditor");
        QDir().mkpath(bookmarkFile);
        bookmarkFile += QStringLiteral("/bookmarks.xml");
    }

    KBookmarkManager *manager = KBookmarkManager::managerForFile(bookmarkFile, QStringLiteral("sieveeditor"));
    manager->setUpdate(true);

    mBookmarkMenu = new KBookmarkMenu(manager, this, mMenu, collection);
}

SieveEditorBookmarks::~SieveEditorBookmarks()
{
    delete mBookmarkMenu;
}

QMenu *SieveEditorBookmarks::menu() const
{
    return mMenu;
}

void SieveEditorBookmarks::openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    Q_UNUSED(mb);
    Q_UNUSED(km);
    Q_EMIT openUrl(bm.url());
}

QString SieveEditorBookmarks::currentTitle() const
{
    return mMainWindow->currentHelpTitle();
}

QUrl SieveEditorBookmarks::currentUrl() const
{
    return mMainWindow->currentHelpUrl();
}

