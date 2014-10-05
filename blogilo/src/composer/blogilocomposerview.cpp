/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "blogilocomposerview.h"
#include "pimcommon/widgets/customtoolswidget.h"

#include <KToggleAction>

#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>

BlogiloComposerView::BlogiloComposerView(QWidget *parent)
    : ComposerEditorNG::ComposerView(parent),
      mCustomTools(0)
{
    settings()->setFontSize(QWebSettings::DefaultFontSize, 14);
}

BlogiloComposerView::~BlogiloComposerView()
{

}

void BlogiloComposerView::setCustomTools(PimCommon::CustomToolsWidget *customTool)
{
    mCustomTools = customTool;
}

void BlogiloComposerView::startEditing()
{
    //NOTE: it needs a mouse click!
    //any better way to make the cursor visible?
    this -> setFocus();
    QMouseEvent mouseEventPress(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(this, &mouseEventPress);
    QTimer::singleShot(50, this, SLOT(slotSendMouseReleaseEvent()));
}

void BlogiloComposerView::slotSendMouseReleaseEvent()
{
    QMouseEvent mouseEventRelease(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(this, &mouseEventRelease);
    pageAction(QWebPage::MoveToEndOfDocument)->trigger();
}

void BlogiloComposerView::addExtraAction(QMenu *menu)
{
    if (mCustomTools) {
        menu->addSeparator();
        menu->addAction(mCustomTools->action(PimCommon::CustomToolsWidget::TranslatorTool));
        menu->addAction(mCustomTools->action(PimCommon::CustomToolsWidget::ShortUrlTool));
    }
}

