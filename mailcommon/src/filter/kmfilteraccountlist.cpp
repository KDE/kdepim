/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "kmfilteraccountlist.h"
#include "mailfilter.h"
#include "util/mailutil.h"

#include <AkonadiCore/AgentInstance>
#include <AkonadiCore/AgentType>

#include <KLocalizedString>

#include <QHeaderView>

using namespace MailCommon;

KMFilterAccountList::KMFilterAccountList(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    QStringList headerNames;
    headerNames << i18n("Account Name") << i18n("Type");
    setHeaderItem(new QTreeWidgetItem(headerNames));
    setAllColumnsShowFocus(true);
    setFrameStyle(QFrame::WinPanel + QFrame::Sunken);
    setSortingEnabled(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    header()->setMovable(false);
}

KMFilterAccountList::~KMFilterAccountList()
{
}

void KMFilterAccountList::updateAccountList(MailCommon::MailFilter *filter)
{
    clear();

    QTreeWidgetItem *top = Q_NULLPTR;
    // Block the signals here, otherwise we end up calling
    // slotApplicableAccountsChanged(), which will read the incomplete item
    // state and write that back to the filter
    blockSignals(true);
    const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
    const int nbAccount = lst.count();
    for (int i = 0; i < nbAccount; ++i) {
        const Akonadi::AgentInstance agent = lst.at(i);
        QTreeWidgetItem *listItem = new QTreeWidgetItem(this, top);
        listItem->setText(0, agent.name());
        listItem->setText(1, agent.type().name());
        listItem->setText(2, agent.identifier());
        if (filter) {
            listItem->setCheckState(0,
                                    filter->applyOnAccount(agent.identifier()) ?
                                    Qt::Checked :
                                    Qt::Unchecked);
        }
        top = listItem;
    }
    blockSignals(false);

    // make sure our hidden column is really hidden (Qt tends to re-show it)
    hideColumn(2);
    resizeColumnToContents(0);
    resizeColumnToContents(1);

    top = topLevelItem(0);
    if (top) {
        setCurrentItem(top);
    }
}

void KMFilterAccountList::applyOnAccount(MailCommon::MailFilter *filter)
{
    QTreeWidgetItemIterator it(this);

    while (QTreeWidgetItem *item = *it) {
        const QString id = item->text(2);
        filter->setApplyOnAccount(id, item->checkState(0) == Qt::Checked);
        ++it;
    }
}

void KMFilterAccountList::applyOnAccount(const QStringList &lstAccount)
{
    clear();

    QTreeWidgetItem *top = Q_NULLPTR;
    // Block the signals here, otherwise we end up calling
    // slotApplicableAccountsChanged(), which will read the incomplete item
    // state and write that back to the filter
    blockSignals(true);
    const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
    const int nbAccount = lst.count();
    for (int i = 0; i < nbAccount; ++i) {
        const Akonadi::AgentInstance agent = lst.at(i);
        QTreeWidgetItem *listItem = new QTreeWidgetItem(this, top);
        listItem->setText(0, agent.name());
        listItem->setText(1, agent.type().name());
        listItem->setText(2, agent.identifier());
        listItem->setCheckState(0, lstAccount.contains(agent.identifier()) ?
                                Qt::Checked : Qt::Unchecked);
        top = listItem;
    }
    blockSignals(false);

    // make sure our hidden column is really hidden (Qt tends to re-show it)
    hideColumn(2);
    resizeColumnToContents(0);
    resizeColumnToContents(1);

    top = topLevelItem(0);
    if (top) {
        setCurrentItem(top);
    }
}

QStringList KMFilterAccountList::selectedAccount()
{
    QStringList lstAccount;
    QTreeWidgetItemIterator it(this);

    while (QTreeWidgetItem *item = *it) {
        if (item->checkState(0) == Qt::Checked) {
            lstAccount << item->text(2);
        }
        ++it;
    }
    return lstAccount;
}
