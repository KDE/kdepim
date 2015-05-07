/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
#include <QWebEnginePage>
#include <QContextMenuEvent>

using namespace KSieveUi;

SieveEditorWebView::SieveEditorWebView(QWidget *parent)
    : QWebEngineView(parent)
{
}

SieveEditorWebView::~SieveEditorWebView()
{
}

void SieveEditorWebView::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu;
    QAction *act = pageAction(QWebEnginePage::Back);
    if (act->isEnabled()) {
        menu.addAction(act);
    }
    act = pageAction(QWebEnginePage::Forward);
    if (act->isEnabled()) {
        menu.addAction(act);
    }

    if (!menu.actions().isEmpty()) {
        QAction *separator = new QAction(&menu);
        separator->setSeparator(true);
        menu.addAction(separator);
    }

    act = pageAction(QWebEnginePage::Copy);
    if (act->isEnabled()) {
        menu.addAction(act);
    }

    if (!menu.actions().isEmpty()) {
        QAction *separator = new QAction(&menu);
        separator->setSeparator(true);
        menu.addAction(separator);
    }
    act = pageAction(QWebEnginePage::Reload);
    if (act->isEnabled()) {
        menu.addAction(act);
    }
    menu.exec(ev->globalPos());
}

