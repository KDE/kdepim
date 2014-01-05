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

#include "webpage.h"
#include "networkaccessmanager.h"
#include "adblockmanager.h"

using namespace MessageViewer;

WebPage::WebPage(QWidget *parent)
    : KWebPage(parent)
{
    MessageViewer::MyNetworkAccessManager *manager = new MessageViewer::MyNetworkAccessManager(this);
    manager->setEmitReadyReadOnMetaDataChange(true);
    manager->setCache(0);
    QWidget* window = parent ? parent->window() : 0;
    if (window) {
        manager->setWindow(window);
    }
    setNetworkAccessManager(manager);
    connect(this, SIGNAL(frameCreated(QWebFrame*)), AdBlockManager::self(), SLOT(applyHidingRules(QWebFrame*)));
}

WebPage::~WebPage()
{
}

KUrl WebPage::loadingUrl() const
{
    return mLoadingUrl;
}

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    const bool isMainFrameRequest = (frame == mainFrame());
    if (isMainFrameRequest) {
        mLoadingUrl = request.url();
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}
