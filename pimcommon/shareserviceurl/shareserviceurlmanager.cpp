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


#include "shareserviceurlmanager.h"

#include <KActionMenu>
#include <KLocalizedString>
#include <QMenu>
#include <QDesktopServices>

using namespace PimCommon;

ShareServiceUrlManager::ShareServiceUrlManager(QObject *parent)
    : QObject(parent),
      mMenu(Q_NULLPTR)
{
    initializeMenu();
}

ShareServiceUrlManager::~ShareServiceUrlManager()
{

}

KActionMenu *ShareServiceUrlManager::menu() const
{
    return mMenu;
}

QString ShareServiceUrlManager::typeToI18n(ServiceType type)
{
    QString str;
    switch (type) {
    case Fbook:
        str = i18n("Facebook");
        break;
    case Twitter:
        str = i18n("Twitter");
        break;
    case GooglePlus:
        str = i18n("Google Plus");
        break;
    case MailTo:
        str = i18n("Mail");
        break;
    case ServiceEndType:
        break;
    }
    return str;
}

void ShareServiceUrlManager::initializeMenu()
{
    mMenu = new KActionMenu(i18n("Share On..."), this);
    for (int i = 0; i < ServiceEndType; ++i) {
        const ServiceType type = static_cast<ServiceType>(i);
        QAction *action = new QAction(typeToI18n(type), this);
        action->setData(type);
        mMenu->addAction(action);
    }
    connect(mMenu->menu(), &QMenu::triggered, this, &ShareServiceUrlManager::slotSelectServiceUrl);
}

void ShareServiceUrlManager::slotSelectServiceUrl(QAction *act)
{
    if (act) {
        const ServiceType type = act->data().value<ServiceType>();
        Q_EMIT serviceUrlSelected(type);
    }
}

QUrl ShareServiceUrlManager::generateServiceUrl(const QString &link, const QString &title, ServiceType type)
{
    QUrl url;
    if (link.isEmpty()) {
        return url;
    }
    switch (type) {
    case Fbook: {
        url.setUrl(QStringLiteral("https://www.facebook.com/sharer.php"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("u"), link);
        urlQuery.addQueryItem(QStringLiteral("t"), title);
        url.setQuery(urlQuery);
        break;
    }
    case Twitter: {
        url.setUrl(QStringLiteral("https://twitter.com/share"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        urlQuery.addQueryItem(QStringLiteral("text"), title);
        url.setQuery(urlQuery);
        break;
    }
    case GooglePlus: {
        url.setUrl(QStringLiteral("https://plus.google.com/share"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("url"), link);
        url.setQuery(urlQuery);
        break;
    }
    case MailTo: {
        url.setUrl(QStringLiteral("mailto:"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QStringLiteral("subject"), title);
        urlQuery.addQueryItem(QStringLiteral("body"), link);
        url.setQuery(urlQuery);
    }
    case ServiceEndType:
        break;
    }
    //TODO
    return url;
}

void ShareServiceUrlManager::openUrl(const QUrl &url)
{
    if (url.isValid()) {
        QDesktopServices::openUrl(url);
    }
}
