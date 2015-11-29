/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "sieveeditorhelphtmlwidget.h"
#include "sieveeditorloadprogressindicator.h"
#include "sieveeditorwebview.h"
#include <QVBoxLayout>

using namespace KSieveUi;
namespace {
qreal zoomBy()
{
    return 20;
}
}
SieveEditorHelpHtmlWidget::SieveEditorHelpHtmlWidget(QWidget *parent)
    : QWidget(parent),
      mZoomFactor(100)
{
    mProgressIndicator = new SieveEditorLoadProgressIndicator(this);
    connect(mProgressIndicator, &SieveEditorLoadProgressIndicator::pixmapChanged, this, &SieveEditorHelpHtmlWidget::slotPixmapChanged);
    connect(mProgressIndicator, &SieveEditorLoadProgressIndicator::loadFinished, this, &SieveEditorHelpHtmlWidget::slotLoadFinished);
    mWebView = new SieveEditorWebView;
    connect(mWebView, &SieveEditorWebView::titleChanged, this, &SieveEditorHelpHtmlWidget::slotTitleChanged);
    connect(mWebView, &SieveEditorWebView::loadStarted, this, &SieveEditorHelpHtmlWidget::slotLoadStarted);
    connect(mWebView, &SieveEditorWebView::loadFinished, this, &SieveEditorHelpHtmlWidget::slotFinished);
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(mWebView);
    setLayout(lay);
}

SieveEditorHelpHtmlWidget::~SieveEditorHelpHtmlWidget()
{

}

QString SieveEditorHelpHtmlWidget::title() const
{
    return mTitle;
}

void SieveEditorHelpHtmlWidget::slotLoadFinished(bool success)
{
    Q_EMIT loadFinished(this, success);
}

QUrl SieveEditorHelpHtmlWidget::currentUrl() const
{
    return mWebView->url();
}

void SieveEditorHelpHtmlWidget::slotPixmapChanged(const QPixmap &pixmap)
{
    Q_EMIT progressIndicatorPixmapChanged(this, pixmap);
}

void SieveEditorHelpHtmlWidget::slotFinished(bool b)
{
    mProgressIndicator->stopAnimation(b);
}

void SieveEditorHelpHtmlWidget::slotLoadStarted()
{
    mProgressIndicator->startAnimation();
}

void SieveEditorHelpHtmlWidget::slotTitleChanged(const QString &title)
{
    if (mTitle != title) {
        mTitle = title;
        Q_EMIT titleChanged(this, title);
    }
}

void SieveEditorHelpHtmlWidget::openUrl(const QUrl &url)
{
    mWebView->setUrl(url);
}

void SieveEditorHelpHtmlWidget::zoomIn()
{
    if (mZoomFactor >= 300) {
        return;
    }
    mZoomFactor += zoomBy();
    if (mZoomFactor > 300) {
        mZoomFactor = 300;
    }
    mWebView->setZoomFactor(mZoomFactor / 100.0);
}

void SieveEditorHelpHtmlWidget::zoomOut()
{
    if (mZoomFactor <= 10) {
        return;
    }
    mZoomFactor -= zoomBy();
    if (mZoomFactor < 10) {
        mZoomFactor = 10;
    }
    mWebView->setZoomFactor(mZoomFactor / 100.0);
}

void SieveEditorHelpHtmlWidget::resetZoom()
{
    mZoomFactor = 100;
    mWebView->setZoomFactor(1.0);
}
