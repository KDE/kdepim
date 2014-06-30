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

#include "followupremindernoanswerdialog.h"
#include "followupreminderinfo.h"

#include <KLocalizedString>
#include <KMenu>
#include <KSharedConfig>

#include <QHBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>


FollowUpReminderNoAnswerDialog::FollowUpReminderNoAnswerDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Follow Up Mail") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Ok|Cancel );
    mWidget = new FollowUpReminderNoAnswerWidget;
    setMainWidget(mWidget);
    readConfig();
}

FollowUpReminderNoAnswerDialog::~FollowUpReminderNoAnswerDialog()
{
    writeConfig();
}

void FollowUpReminderNoAnswerDialog::setInfo(const QList<FollowUpReminderInfo *> &info)
{
    mWidget->setInfo(info);
}

void FollowUpReminderNoAnswerDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "FollowUpReminderNoAnswerDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void FollowUpReminderNoAnswerDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "FollowUpReminderNoAnswerDialog" );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
}



FollowUpReminderNoAnswerWidget::FollowUpReminderNoAnswerWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    QLabel *lab = new QLabel(i18n("You still wait an answer about this mail:"));
    vbox->addWidget(lab);
    mTreeWidget = new QTreeWidget;
    //TODO
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Message Id")
            << i18n("Date");

    mTreeWidget->setHeaderLabels(headers);
    mTreeWidget->setSortingEnabled(true);
    mTreeWidget->setRootIsDecorated(false);
    mTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    vbox->addWidget(mTreeWidget);
    setLayout(vbox);
}

FollowUpReminderNoAnswerWidget::~FollowUpReminderNoAnswerWidget()
{
}

void FollowUpReminderNoAnswerWidget::customContextMenuRequested(const QPoint &pos)
{
    const QList<QTreeWidgetItem *> listItems = mTreeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        KMenu menu;
        menu.addAction(KIcon(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void FollowUpReminderNoAnswerWidget::setInfo(const QList<FollowUpReminderInfo *> &info)
{
    //TODO
}


void FollowUpReminderNoAnswerWidget::slotRemoveItem()
{
    //TODO
}

void FollowUpReminderNoAnswerWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mTreeWidget->header()->restoreState(data);
}

void FollowUpReminderNoAnswerWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mTreeWidget->header()->saveState() );
}

