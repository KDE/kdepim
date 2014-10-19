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

#include "sendlaterconfigurewidget.h"
#include "sendlaterinfo.h"
#include "sendlaterutil.h"
#include "sendlaterdialog.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KHelpMenu>
#include <KMenu>
#include <KMessageBox>
#include <KIcon>
#include <KGlobal>
#include <QPointer>

static QString sendLaterItemPattern = QLatin1String( "SendLaterItem \\d+" );

//#define DEBUG_MESSAGE_ID

SendLaterItem::SendLaterItem(QTreeWidget *parent )
    : QTreeWidgetItem(parent),
      mInfo(0)
{
}

SendLaterItem::~SendLaterItem()
{
    delete mInfo;
}

void SendLaterItem::setInfo(SendLater::SendLaterInfo *info)
{
    mInfo = info;
}

SendLater::SendLaterInfo* SendLaterItem::info() const
{
    return mInfo;
}

SendLaterWidget::SendLaterWidget( QWidget *parent )
    : QWidget( parent ),
      mChanged(false)
{
    mWidget = new Ui::SendLaterConfigureWidget;
    mWidget->setupUi( this );
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Send around")
            << i18n("Recurrent")
#ifdef DEBUG_MESSAGE_ID
            << i18n("Message Id");
#else
               ;
#endif

    mWidget->treeWidget->setObjectName(QLatin1String("treewidget"));
    mWidget->treeWidget->setHeaderLabels(headers);
    mWidget->treeWidget->setSortingEnabled(true);
    mWidget->treeWidget->setRootIsDecorated(false);
    mWidget->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mWidget->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    mWidget->treeWidget->setDefaultText(i18n("No messages waiting..."));

    connect(mWidget->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    connect(mWidget->removeItem, SIGNAL(clicked(bool)), SLOT(slotRemoveItem()));
    connect(mWidget->modifyItem, SIGNAL(clicked(bool)), SLOT(slotModifyItem()));
    connect(mWidget->treeWidget, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()));
    connect(mWidget->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotModifyItem()));
    load();
    updateButtons();
}

SendLaterWidget::~SendLaterWidget()
{
    delete mWidget;
}

void SendLaterWidget::customContextMenuRequested(const QPoint &)
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        KMenu menu;
        if ( listItems.count() == 1) {
            menu.addAction(i18n("Send now"), this, SLOT(slotSendNow()));
        }
        menu.addSeparator();
        menu.addAction(KIcon(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void SendLaterWidget::slotSendNow()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count() == 1) {
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(listItems.first());
        Q_EMIT sendNow(mailItem->info()->itemId());
    }
}

void SendLaterWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mWidget->treeWidget->header()->restoreState(data);
}

void SendLaterWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mWidget->treeWidget->header()->saveState() );
}

void SendLaterWidget::updateButtons()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.isEmpty()) {
        mWidget->removeItem->setEnabled(false);
        mWidget->modifyItem->setEnabled(false);
    } else if (listItems.count() == 1) {
        mWidget->removeItem->setEnabled(true);
        mWidget->modifyItem->setEnabled(true);
    } else {
        mWidget->removeItem->setEnabled(true);
        mWidget->modifyItem->setEnabled(false);
    }
}

void SendLaterWidget::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    const QStringList filterGroups = config->groupList().filter( QRegExp( sendLaterItemPattern ) );
    const int numberOfItem = filterGroups.count();
    for (int i = 0 ; i < numberOfItem; ++i) {
        KConfigGroup group = config->group(filterGroups.at(i));
        SendLater::SendLaterInfo *info = new SendLater::SendLaterInfo(group);
        createOrUpdateItem(info);
    }
    mWidget->treeWidget->setShowDefaultText(numberOfItem==0);
}

void SendLaterWidget::createOrUpdateItem(SendLater::SendLaterInfo *info, SendLaterItem *item)
{
    if (!item) {
        item = new SendLaterItem(mWidget->treeWidget);
    }
    item->setText(Recursive, info->isRecurrence() ? i18n("Yes") : i18n("No"));
#ifdef DEBUG_MESSAGE_ID
    item->setText(MessageId, QString::number(info->itemId()));
#endif
    item->setText(SendAround, info->dateTime().toString());
    item->setText(Subject, info->subject());
    item->setText(To, info->to());
    item->setInfo(info);
    mWidget->treeWidget->setShowDefaultText(false);
}

void SendLaterWidget::save()
{
    if (!mChanged)
        return;
    KSharedConfig::Ptr config = KGlobal::config();

    // first, delete all filter groups:
    const QStringList filterGroups =config->groupList().filter( QRegExp( sendLaterItemPattern ) );

    foreach ( const QString &group, filterGroups ) {
        config->deleteGroup( group );
    }

    const int numberOfItem(mWidget->treeWidget->topLevelItemCount());
    for (int i = 0; i < numberOfItem; ++i) {
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(mWidget->treeWidget->topLevelItem(i));
        if (mailItem->info()) {
            KConfigGroup group = config->group(SendLater::SendLaterUtil::sendLaterPattern.arg(mailItem->info()->itemId()));
            mailItem->info()->writeConfig(group);
        }
    }
    config->sync();
    config->reparseConfiguration();
}

void SendLaterWidget::slotRemoveItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (KMessageBox::warningYesNo(this,i18n("Do you want to delete selected items? Do you want to continue?"),i18n("Remove items"))== KMessageBox::No)
        return;

    bool removeMessage = false;
    if (KMessageBox::warningYesNo(this,i18n("Do you want to remove messages as well?"),i18n("Remove messages"))== KMessageBox::Yes)
        removeMessage = true;

    Q_FOREACH(QTreeWidgetItem *item,listItems) {
        if (removeMessage) {
            SendLaterItem *mailItem = static_cast<SendLaterItem *>(item);
            if (mailItem->info()) {
                Akonadi::Item::Id id = mailItem->info()->itemId();
                if (id != -1)
                    mListMessagesToRemove << id;
            }
        }
        delete item;
    }
    mChanged = true;
    mWidget->treeWidget->setShowDefaultText(mWidget->treeWidget->topLevelItemCount()==0);
    updateButtons();
}

void SendLaterWidget::slotModifyItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count()==1) {
        QTreeWidgetItem *item = listItems.first();
        if (!item)
            return;
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(item);

        QPointer<SendLater::SendLaterDialog> dialog = new SendLater::SendLaterDialog(mailItem->info(), this);
        if (dialog->exec()) {
            SendLater::SendLaterInfo *info = dialog->info();
            createOrUpdateItem(info, mailItem);
            mChanged = true;
        }
        delete dialog;
    }
}

void SendLaterWidget::needToReload()
{
    mWidget->treeWidget->clear();
    KSharedConfig::Ptr config = KGlobal::config();
    config->reparseConfiguration();
    load();
}

QList<Akonadi::Item::Id> SendLaterWidget::messagesToRemove() const
{
    return mListMessagesToRemove;
}
