/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

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

#include "blogilocomposerwebengineview.h"
#include "PimCommon/CustomToolsWidgetng"

#include <KToggleAction>

#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>

BlogiloComposerWebEngineView::BlogiloComposerWebEngineView(QWidget *parent)
    : ComposerEditorNG::ComposerView(parent),
      mCustomTools(Q_NULLPTR)
{
    settings()->setFontSize(QWebSettings::DefaultFontSize, 14);
}

BlogiloComposerWebEngineView::~BlogiloComposerWebEngineView()
{

}

void BlogiloComposerWebEngineView::setCustomTools(PimCommon::CustomToolsWidgetNg *customTool)
{
    mCustomTools = customTool;
}

void BlogiloComposerWebEngineView::startEditing()
{
    //NOTE: it needs a mouse click!
    //any better way to make the cursor visible?
    this -> setFocus();
    QMouseEvent mouseEventPress(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(this, &mouseEventPress);
    QTimer::singleShot(50, this, &BlogiloComposerWebEngineView::slotSendMouseReleaseEvent);
}

void BlogiloComposerWebEngineView::slotSendMouseReleaseEvent()
{
    QMouseEvent mouseEventRelease(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(this, &mouseEventRelease);
    pageAction(QWebPage::MoveToEndOfDocument)->trigger();
}

void BlogiloComposerWebEngineView::addExtraAction(QMenu *menu)
{
    if (mCustomTools) {
        menu->addSeparator();
        Q_FOREACH (KToggleAction *ta, mCustomTools->actionList()) {
            menu->addAction(ta);
        }
    }
}

