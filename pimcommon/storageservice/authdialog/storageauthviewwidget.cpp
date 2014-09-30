/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "storageauthviewwidget.h"
#include "storageauthwebview.h"

#include <QVBoxLayout>
#include <QProgressBar>

using namespace PimCommon;

StorageAuthViewWidget::StorageAuthViewWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    mWebView = new StorageAuthWebView;
    mProgressBar = new QProgressBar;
    mProgressBar->hide();
    connect(mWebView, &StorageAuthWebView::urlChanged, this, &StorageAuthViewWidget::urlChanged);
    connect(mWebView, &StorageAuthWebView::loadStarted, this, &StorageAuthViewWidget::slotLoadStarted);
    connect(mWebView, &StorageAuthWebView::loadProgress, mProgressBar, &QProgressBar::setValue);
    connect(mWebView, &StorageAuthWebView::loadFinished, this, &StorageAuthViewWidget::slotLoadFinished);
    lay->addWidget(mWebView);
    lay->addWidget(mProgressBar);
    setLayout(lay);
}

StorageAuthViewWidget::~StorageAuthViewWidget()
{

}

void StorageAuthViewWidget::setUrl(const QUrl &url)
{
    mWebView->load(url);
}

void StorageAuthViewWidget::slotLoadStarted()
{
    mProgressBar->show();
}

void StorageAuthViewWidget::slotLoadFinished(bool success)
{
    mProgressBar->hide();
}
