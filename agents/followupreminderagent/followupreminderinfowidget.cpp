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

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include <kicon.h>
#include <KGlobal>
#include <KLocale>
#include <kmenu.h>
#include <KLocalizedString>

//#define DEBUG_MESSAGE_ID
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

FollowUpReminder::FollowUpReminderInfo* FollowUpReminderInfoItem::info() const
{
    return mInfo;
}

FollowUpReminderInfoWidget::FollowUpReminderInfoWidget(QWidget *parent)
    : QWidget(parent),
      mChanged(false)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    mTreeWidget = new QTreeWidget;
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Dead Line")
            << i18n("Answer")
#ifdef DEBUG_MESSAGE_ID
            << i18n("Message Id");
#else
               ;
#endif

    mTreeWidget->setHeaderLabels(headers);
    mTreeWidget->setSortingEnabled(true);
    mTreeWidget->setRootIsDecorated(false);
    mTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

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
        FollowUpReminderInfoItem *item = new FollowUpReminderInfoItem(mTreeWidget);
        item->setData(0, AnswerItemId, info->answerMessageItemId());
        item->setData(0, AnswerItemFound, info->answerWasReceived());
        item->setText(To, info->to());
#ifdef DEBUG_MESSAGE_ID
        item->setText(MessageId, info->messageId());
#endif
        item->setText(Subject, info->subject());
        const QString date = KGlobal::locale()->formatDateTime( info->followUpReminderDate(), KLocale::LongDate );
        item->setText(DeadLine, date);
        item->setText(AnswerWasReceived, info->answerWasReceived() ? i18n("Received") : i18n("On hold"));
    }
}

void FollowUpReminderInfoWidget::save()
{
    if (!mChanged)
        return;
    KSharedConfig::Ptr config = KGlobal::config();

    // first, delete all filter groups:
    const QStringList filterGroups =config->groupList().filter( QRegExp( followUpItemPattern ) );

    foreach ( const QString &group, filterGroups ) {
        config->deleteGroup( group );
    }

    const int numberOfItem(mTreeWidget->topLevelItemCount());
    for (int i = 0; i < numberOfItem; ++i) {
        FollowUpReminderInfoItem *mailItem = static_cast<FollowUpReminderInfoItem *>(mTreeWidget->topLevelItem(i));
        if (mailItem->info()) {
            KConfigGroup group = config->group(QString::fromLatin1("FollowupReminderItem %1").arg(mailItem->info()->id()));
            mailItem->info()->writeConfig(group);
        }
    }
    config->sync();
    config->reparseConfiguration();
}

void FollowUpReminderInfoWidget::customContextMenuRequested(const QPoint &pos)
{
    const QList<QTreeWidgetItem *> listItems = mTreeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        FollowUpReminderInfoItem *mailItem = static_cast<FollowUpReminderInfoItem *>(listItems.at(0));
        KMenu menu;
        QAction *showMessage = 0;
        if (mailItem && mailItem->data(0, AnswerItemFound).toBool()) {
            showMessage = menu.addAction(i18n("Show Message"));
        }
        QAction *deleteItem = menu.addAction(KIcon(QLatin1String("edit-delete")), i18n("Delete"));
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
    //TODO
}

void FollowUpReminderInfoWidget::removeItem(FollowUpReminderInfoItem *mailItem)
{
    if (mailItem) {
        delete mailItem;
        mChanged = true;
    }
}

void FollowUpReminderInfoWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mTreeWidget->header()->restoreState(data);
}

void FollowUpReminderInfoWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mTreeWidget->header()->saveState() );
}

