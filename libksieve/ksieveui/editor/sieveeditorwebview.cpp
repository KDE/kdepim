/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "sieveeditorwebview.h"
#include <QMenu>
#include <QWebPage>
#include <QContextMenuEvent>

using namespace KSieveUi;

SieveEditorWebView::SieveEditorWebView(QWidget *parent)
    : QWebView(parent)
{
}

SieveEditorWebView::~SieveEditorWebView()
{
}

void SieveEditorWebView::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu;
    QAction *act = pageAction(QWebPage::Back);
    if (act->isEnabled()) {
        menu.addAction(act);
    }
    act = pageAction(QWebPage::Forward);
    if (act->isEnabled()) {
        menu.addAction(act);
    }

    if (menu.actions().count() > 0) {
        QAction *separator = new QAction(&menu);
        separator->setSeparator(true);
        menu.addAction(separator);
    }

    act = pageAction(QWebPage::CopyLinkToClipboard);
    if (act->isEnabled()) {
        menu.addAction(act);
    }

    if (menu.actions().count() > 0) {
        QAction *separator = new QAction(&menu);
        separator->setSeparator(true);
        menu.addAction(separator);
    }
    act = pageAction(QWebPage::Reload);
    if (act->isEnabled()) {
        menu.addAction(act);
    }
    menu.exec(ev->globalPos());
}

