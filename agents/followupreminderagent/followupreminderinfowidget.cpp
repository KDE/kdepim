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
#include <kmenu.h>
#include <KLocalizedString>

FollowUpReminderInfoWidget::FollowUpReminderInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    mTreeWidget = new QTreeWidget;
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Message Id")
            << i18n("Dead Line");

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

void FollowUpReminderInfoWidget::setInfo(const QList<FollowUpReminderInfo *> &info)
{
    //TODO
}

void FollowUpReminderInfoWidget::customContextMenuRequested(const QPoint &pos)
{
    const QList<QTreeWidgetItem *> listItems = mTreeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        KMenu menu;
        menu.addAction(KIcon(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void FollowUpReminderInfoWidget::slotRemoveItem()
{
    //TODO
}

void FollowUpReminderInfoWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mTreeWidget->header()->restoreState(data);
}

void FollowUpReminderInfoWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mTreeWidget->header()->saveState() );
}

