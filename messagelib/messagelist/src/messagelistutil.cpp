/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "messagelistutil.h"
#include "messagelistsettings.h"

#include <KConfigGroup>
#include <QMenu>
#include <QIcon>
#include <KLocalizedString>
#include <KMime/Message>
#include <Item>
#include <KColorScheme>

QString MessageList::Util::messageSortingConfigName()
{
    return QStringLiteral("MessageSorting");
}

QString MessageList::Util::messageSortDirectionConfigName()
{
    return QStringLiteral("MessageSortDirection");
}

QString MessageList::Util::groupSortingConfigName()
{
    return QStringLiteral("GroupSorting");
}

QString MessageList::Util::groupSortDirectionConfigName()
{
    return QStringLiteral("GroupSortDirection");
}

QString MessageList::Util::messageUniqueIdConfigName()
{
    return QStringLiteral("MessageUniqueIdForStorageModel%1");
}

QString MessageList::Util::storageModelSortOrderGroup()
{
    return QStringLiteral("MessageListView::StorageModelSortOrder");
}

QString MessageList::Util::storageModelThemesGroup()
{
    return QStringLiteral("MessageListView::StorageModelThemes");
}

QString MessageList::Util::storageModelAggregationsGroup()
{
    return QStringLiteral("MessageListView::StorageModelAggregations");
}

QString MessageList::Util::setForStorageModelConfigName()
{
    return QStringLiteral("SetForStorageModel%1");
}

QString MessageList::Util::storageModelSelectedMessageGroup()
{
    return QStringLiteral("MessageListView::StorageModelSelectedMessages");
}

void MessageList::Util::deleteConfig(const QString &collectionId)
{
    KConfigGroup confselectedMessage(MessageList::MessageListSettings::self()->config(),
                                     MessageList::Util::storageModelSelectedMessageGroup());
    confselectedMessage.deleteEntry(MessageList::Util::messageUniqueIdConfigName().arg(collectionId));

    KConfigGroup storageModelOrder(MessageListSettings::self()->config(),
                                   MessageList::Util::storageModelSortOrderGroup());
    storageModelOrder.deleteEntry(collectionId + groupSortDirectionConfigName());
    storageModelOrder.deleteEntry(collectionId + groupSortingConfigName());
    storageModelOrder.deleteEntry(collectionId + messageSortDirectionConfigName());
    storageModelOrder.deleteEntry(collectionId + messageSortingConfigName());

    KConfigGroup storageModelTheme(MessageListSettings::self()->config(),
                                   MessageList::Util::storageModelThemesGroup());
    storageModelTheme.deleteEntry(collectionId + setForStorageModelConfigName());

    KConfigGroup storageModelAggregation(MessageListSettings::self()->config(),
                                         MessageList::Util::storageModelAggregationsGroup());
    storageModelAggregation.deleteEntry(collectionId + setForStorageModelConfigName());

}

QColor MessageList::Util::unreadDefaultMessageColor()
{
    return KColorScheme(QPalette::Active).decoration(KColorScheme::FocusColor).color();
}

QColor MessageList::Util::importantDefaultMessageColor()
{
    return KColorScheme(QPalette::Active).foreground(KColorScheme::NegativeText).color();
}

QColor MessageList::Util::todoDefaultMessageColor()
{
    return KColorScheme(QPalette::Active).foreground(KColorScheme::PositiveText).color();
}

void MessageList::Util::fillViewMenu(QMenu *menu, QObject *receiver)
{
    QMenu *sortingMenu = new QMenu(i18n("Sorting"), menu);
    sortingMenu->setIcon(QIcon::fromTheme(QStringLiteral("view-sort-ascending")));
    menu->addMenu(sortingMenu);
    QObject::connect(sortingMenu, SIGNAL(aboutToShow()), receiver, SLOT(sortOrderMenuAboutToShow()));

    QMenu *aggregationMenu = new QMenu(i18n("Aggregation"), menu);
    aggregationMenu->setIcon(QIcon::fromTheme(QStringLiteral("view-process-tree")));
    menu->addMenu(aggregationMenu);
    QObject::connect(aggregationMenu, SIGNAL(aboutToShow()), receiver, SLOT(aggregationMenuAboutToShow()));

    QMenu *themeMenu = new QMenu(i18n("Theme"), menu);
    themeMenu->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-theme")));
    menu->addMenu(themeMenu);
    QObject::connect(themeMenu, SIGNAL(aboutToShow()),
                     receiver, SLOT(themeMenuAboutToShow()));
}

QString MessageList::Util::contentSummary(const Akonadi::Item &item)
{
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return QString();
    }

    KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    KMime::Content *textContent = message->textContent();
    if (!textContent) {
        return QString();
    }
    const QString content = textContent->decodedText(true, true);
    if (content.isEmpty()) {
        return QString();
    }

    // Extract the first 5 non-empty, non-quoted lines from the content and return it
    int numLines = 0;
    const int maxLines = 5;
    const QStringList lines = content.split(QLatin1Char('\n'));
    QString ret;
    foreach (const QString &line, lines) {
        const QString lineTrimmed = line.trimmed();
        const bool isQuoted = lineTrimmed.startsWith(QLatin1Char('>')) || lineTrimmed.startsWith(QLatin1Char('|'));
        if (!isQuoted && !lineTrimmed.isEmpty()) {
            ret += line + QLatin1Char('\n');
            numLines++;
            if (numLines >= maxLines) {
                break;
            }
        }
    }
    return ret.toHtmlEscaped();
}

