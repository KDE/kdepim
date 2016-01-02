/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "poststabwidget.h"

#include <KLocalizedString>
#include <QIcon>
#include <QMenu>

#include <QToolButton>
#include <QTabBar>

PostsTabWidget::PostsTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setElideMode(Qt::ElideRight);
    setTabsClosable(true);
    tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setDocumentMode(true);

    setMovable(true);

    mNewTabButton = new QToolButton(this);
    mNewTabButton->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    mNewTabButton->adjustSize();
    mNewTabButton->setToolTip(i18nc("@info:tooltip", "Open a new tab"));
#ifndef QT_NO_ACCESSIBILITY
    mNewTabButton->setAccessibleName(i18n("New tab"));
#endif
    setCornerWidget(mNewTabButton, Qt::TopLeftCorner);
    connect(mNewTabButton, &QToolButton::clicked, this, &PostsTabWidget::createNewPost);

    mCloseTabButton = new QToolButton(this);
    mCloseTabButton->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    mCloseTabButton->adjustSize();
    mCloseTabButton->setToolTip(i18nc("@info:tooltip", "Close the current tab"));
#ifndef QT_NO_ACCESSIBILITY
    mCloseTabButton->setAccessibleName(i18n("Close tab"));
#endif
    setCornerWidget(mCloseTabButton, Qt::TopRightCorner);
    connect(mCloseTabButton, &QToolButton::clicked, this, &PostsTabWidget::closeTabClicked);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &PostsTabWidget::customContextMenuRequested, this, &PostsTabWidget::slotTabContextMenuRequest);
}

PostsTabWidget::~PostsTabWidget()
{
}

void PostsTabWidget::slotTabContextMenuRequest(const QPoint &pos)
{
    QTabBar *bar = tabBar();
    if (count() < 1) {
        return;
    }

    const int indexBar = bar->tabAt(bar->mapFrom(this, pos));
    if (indexBar == -1) {
        return;
    }

    QMenu menu(this);
    QAction *closeTab = menu.addAction(i18nc("@action:inmenu", "Close Tab"));
    closeTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));

    QAction *allOther = menu.addAction(i18nc("@action:inmenu", "Close All Other Tabs"));
    allOther->setEnabled(count() > 1);
    allOther->setIcon(QIcon::fromTheme(QStringLiteral("tab-close-other")));

    QAction *action = menu.exec(mapToGlobal(pos));

    if (action == allOther) {   // Close all other tabs
        Q_EMIT tabRemoveAllExclude(indexBar);
    } else if (action == closeTab) {
        Q_EMIT tabCloseRequested(indexBar);
    }
}

