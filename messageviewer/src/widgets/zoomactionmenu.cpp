/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "zoomactionmenu.h"
#include <KLocalizedString>
#include <KToggleAction>
#include <KActionCollection>
#include <viewer/mailwebview.h>

using namespace MessageViewer;
namespace
{
qreal zoomBy()
{
    return 20;
}
}

ZoomActionMenu::ZoomActionMenu(MessageViewer::MailWebView *mailViewer, QObject *parent)
    : KActionMenu(parent),
      mZoomFactor(100),
      mZoomTextOnlyAction(Q_NULLPTR),
      mZoomInAction(Q_NULLPTR),
      mZoomOutAction(Q_NULLPTR),
      mZoomResetAction(Q_NULLPTR),
      mActionCollection(Q_NULLPTR),
      mMailWebViewer(mailViewer),
      mZoomTextOnly(false)
{
}

ZoomActionMenu::~ZoomActionMenu()
{

}

void ZoomActionMenu::setActionCollection(KActionCollection *ac)
{
    mActionCollection = ac;
}

void ZoomActionMenu::createZoomActions()
{
    // Zoom actions
    mZoomTextOnlyAction = new KToggleAction(i18n("Zoom Text Only"), this);
    mActionCollection->addAction(QStringLiteral("toggle_zoomtextonly"), mZoomTextOnlyAction);
    connect(mZoomTextOnlyAction, &QAction::triggered, this, &ZoomActionMenu::slotZoomTextOnly);

    mZoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("&Zoom In"), this);
    mActionCollection->addAction(QStringLiteral("zoom_in"), mZoomInAction);
    connect(mZoomInAction, &QAction::triggered, this, &ZoomActionMenu::slotZoomIn);
    mActionCollection->setDefaultShortcut(mZoomInAction, QKeySequence(Qt::CTRL | Qt::Key_Plus));

    mZoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom &Out"), this);
    mActionCollection->addAction(QStringLiteral("zoom_out"), mZoomOutAction);
    connect(mZoomOutAction, &QAction::triggered, this, &ZoomActionMenu::slotZoomOut);
    mActionCollection->setDefaultShortcut(mZoomOutAction, QKeySequence(Qt::CTRL | Qt::Key_Minus));

    mZoomResetAction = new QAction(i18n("Reset"), this);
    mActionCollection->addAction(QStringLiteral("zoom_reset"), mZoomResetAction);
    connect(mZoomResetAction, &QAction::triggered, this, &ZoomActionMenu::slotZoomReset);
    mActionCollection->setDefaultShortcut(mZoomResetAction, QKeySequence(Qt::CTRL | Qt::Key_0));

}

KToggleAction *ZoomActionMenu::zoomTextOnlyAction() const
{
    return mZoomTextOnlyAction;
}

QAction *ZoomActionMenu::zoomInAction() const
{
    return mZoomInAction;
}

QAction *ZoomActionMenu::zoomOutAction() const
{
    return mZoomOutAction;
}

QAction *ZoomActionMenu::zoomResetAction() const
{
    return mZoomResetAction;
}

void ZoomActionMenu::setZoomFactor(qreal zoomFactor)
{
    mZoomFactor = zoomFactor;
}

void ZoomActionMenu::setWebViewerZoomFactor(qreal zoomFactor)
{
    mMailWebViewer->setZoomFactor(zoomFactor);
}

void ZoomActionMenu::slotZoomIn()
{
    if (mZoomFactor >= 300) {
        return;
    }
    mZoomFactor += zoomBy();
    if (mZoomFactor > 300) {
        mZoomFactor = 300;
    }
    mMailWebViewer->setZoomFactor(mZoomFactor / 100.0);
}

void ZoomActionMenu::slotZoomOut()
{
    if (mZoomFactor <= 10) {
        return;
    }
    mZoomFactor -= zoomBy();
    if (mZoomFactor < 10) {
        mZoomFactor = 10;
    }
    mMailWebViewer->setZoomFactor(mZoomFactor / 100.0);
}

void ZoomActionMenu::setZoomTextOnly(bool textOnly)
{
    mZoomTextOnly = textOnly;
    if (mZoomTextOnlyAction) {
        mZoomTextOnlyAction->setChecked(mZoomTextOnly);
    }
    mMailWebViewer->settings()->setAttribute(QWebSettings::ZoomTextOnly, mZoomTextOnly);
}

void ZoomActionMenu::slotZoomTextOnly()
{
    setZoomTextOnly(!mZoomTextOnly);
}

void ZoomActionMenu::slotZoomReset()
{
    mZoomFactor = 100;
    mMailWebViewer->setZoomFactor(1.0);
}

bool ZoomActionMenu::zoomTextOnly() const
{
    return mZoomTextOnly;
}

qreal ZoomActionMenu::zoomFactor() const
{
    return mZoomFactor;
}
