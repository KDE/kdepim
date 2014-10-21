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
#include "followupreminderinfowidget.h"
#include "followupreminderinfo.h"
#include "followupreminderutil.h"
#include "jobs/followupremindershowmessagejob.h"

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include <QIcon>
#include <QMenu>
#include <KGlobal>
#include <KLocalizedString>
#include <KLocalizedString>
#include <KSharedConfig>

// #define DEBUG_MESSAGE_ID
static QString followUpItemPattern = QLatin1String("FollowupReminderItem \\d+");

FollowUpReminderInfoItem::FollowUpReminderInfoItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent),
      mInfo(0)
{
}

FollowUpReminderInfoItem::~FollowUpReminderInfoItem()
{
    delete mInfo;
}

void FollowUpReminderInfoItem::setInfo(FollowUpReminder::FollowUpReminderInfo *info)
{
    mInfo = info;
}

FollowUpReminder::FollowUpReminderInfo *FollowUpReminderInfoItem::info() const
{
    return mInfo;
}

FollowUpReminderInfoWidget::FollowUpReminderInfoWidget(QWidget *parent)
    : QWidget(parent),
      mChanged(false)
{
    setObjectName(QLatin1String("FollowUpReminderInfoWidget"));
    QHBoxLayout *hbox = new QHBoxLayout;
    mTreeWidget = new QTreeWidget;
    mTreeWidget->setObjectName(QLatin1String("treewidget"));
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Dead Line")
            << i18n("Answer")
#ifdef DEBUG_MESSAGE_ID
            << QLatin1String("Message Id")
            << QLatin1String("Answer Message Id")
#endif
            ;

    mTreeWidget->setHeaderLabels(headers);
    mTreeWidget->setSortingEnabled(true);
    mTreeWidget->setRootIsDecorated(false);
    mTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeWidget, &QTreeWidget::customContextMenuRequested, this, &FollowUpReminderInfoWidget::customContextMenuRequested);

    hbox->addWidget(mTreeWidget);
    setLayout(hbox);
}

FollowUpReminderInfoWidget::~FollowUpReminderInfoWidget()
{
}

void FollowUpReminderInfoWidget::setInfo(const QList<FollowUpReminder::FollowUpReminderInfo *> &infoList)
{
    mTreeWidget->clear();
    Q_FOREACH(FollowUpReminder::FollowUpReminderInfo *info, infoList) {
        if (info->isValid())
            createOrUpdateItem(info);
    }
}

void FollowUpReminderInfoWidget::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    const QStringList filterGroups =config->groupList().filter( QRegExp( followUpItemPattern ) );
    const int numberOfItem = filterGroups.count();
    for (int i = 0 ; i < numberOfItem; ++i) {
        KConfigGroup group = config->group(filterGroups.at(i));

        FollowUpReminder::FollowUpReminderInfo *info = new FollowUpReminder::FollowUpReminderInfo(group);
        if (info->isValid())
            createOrUpdateItem(info);
        else
            delete info;
    }
}

void FollowUpReminderInfoWidget::createOrUpdateItem(FollowUpReminder::FollowUpReminderInfo *info, FollowUpReminderInfoItem *item)
{
    if (!item) {
        item = new FollowUpReminderInfoItem(mTreeWidget);
    }

    item->setText(To, info->to());
    item->setText(Subject, info->subject());
    const QString date = KGlobal::locale()->formatDate( info->followUpReminderDate(), KLocale::LongDate );
    item->setText(DeadLine, date);
    item->setText(AnswerWasReceived, info->answerWasReceived() ? i18n("Received") : i18n("On hold"));
    if (info->followUpReminderDate() < QDate::currentDate()) {
        item->setBackgroundColor(DeadLine, Qt::red);
    }
    item->setData(0, AnswerItemFound, info->answerWasReceived());
#ifdef DEBUG_MESSAGE_ID
    item->setText(MessageId, QString::number(info->originalMessageItemId()));
    item->setText(AnswerMessageId, QString::number(info->answerMessageItemId()));
#endif
}



void FollowUpReminderInfoWidget::save()
{
    if (!mChanged) {
        return;
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    // first, delete all filter groups:
    const QStringList filterGroups = config->groupList().filter(QRegExp(followUpItemPattern));

    foreach (const QString &group, filterGroups) {
        config->deleteGroup(group);
    }

    const int numberOfItem(mTreeWidget->topLevelItemCount());
    int i = 0;
    for (; i < numberOfItem; ++i) {
        FollowUpReminderInfoItem *mailItem = static_cast<FollowUpReminderInfoItem *>(mTreeWidget->topLevelItem(i));
        if (mailItem->info()) {
            KConfigGroup group = config->group(FollowUpReminder::FollowUpReminderUtil::followUpReminderPattern.arg(i));
            mailItem->info()->writeConfig(group);
        }
    }
    ++i;
    KConfigGroup general = config->group(QLatin1String("General"));
    general.writeEntry("Number", i);
    config->sync();
    config->reparseConfiguration();
}

void FollowUpReminderInfoWidget::customContextMenuRequested(const QPoint &pos)
{
    const QList<QTreeWidgetItem *> listItems = mTreeWidget->selectedItems();
    if (!listItems.isEmpty()) {
        FollowUpReminderInfoItem *mailItem = static_cast<FollowUpReminderInfoItem *>(listItems.at(0));
        QMenu menu;
        QAction *showMessage = 0;
        if (mailItem && mailItem->data(0, AnswerItemFound).toBool()) {
            showMessage = menu.addAction(i18n("Show Message"));
        }
        QAction *deleteItem = menu.addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"));
        QAction *result = menu.exec(QCursor::pos());
        if (result) {
            if (result == showMessage) {
                openShowMessage(mailItem->data(0, AnswerItemId).toLongLong());
            } else if (result == deleteItem) {
                removeItem(mailItem);
            }
        }
    }
}

void FollowUpReminderInfoWidget::openShowMessage(Akonadi::Item::Id id)
{
    FollowUpReminderShowMessageJob *job = new FollowUpReminderShowMessageJob(id);
    job->start();
}

void FollowUpReminderInfoWidget::removeItem(FollowUpReminderInfoItem *mailItem)
{
    if (mailItem) {
        delete mailItem;
        mChanged = true;
    } else {
        qDebug() << "Not item selected";
    }
}

void FollowUpReminderInfoWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mTreeWidget->header()->restoreState(data);
}

void FollowUpReminderInfoWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry("HeaderState", mTreeWidget->header()->saveState());
}

