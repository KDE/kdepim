/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "custommanagesievewidget.h"
#include "widgets/managesievetreeview.h"
#include <ksieveui/util.h>

#include <AkonadiCore/AgentInstance>
#include <KLocalizedString>
#include <QIcon>
#include <QTreeWidgetItem>
#include <kmanagesieve/sievejob.h>
#include <widgets/sievetreewidgetitem.h>

using namespace KSieveUi;

CustomManageSieveWidget::CustomManageSieveWidget(QWidget *parent)
    : KSieveUi::ManageSieveWidget(parent)
{

}

CustomManageSieveWidget::~CustomManageSieveWidget()
{

}

bool CustomManageSieveWidget::refreshList()
{
    bool noImapFound = true;
    SieveTreeWidgetItem *last = Q_NULLPTR;
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    Q_FOREACH (const Akonadi::AgentInstance &type, lst) {
        if (type.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        QString serverName = type.name();
        last = new SieveTreeWidgetItem(treeView(), last);
        last->setIcon(0, QIcon::fromTheme(QStringLiteral("network-server")));

        const QUrl u = KSieveUi::Util::findSieveUrlForAccount(type.identifier());
        if (u.isEmpty()) {
            QTreeWidgetItem *item = new QTreeWidgetItem(last);
            item->setText(0, i18n("No Sieve URL configured"));
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            treeView()->expandItem(last);
        } else {
            serverName += QStringLiteral(" (%1)").arg(u.userName());
            KManageSieve::SieveJob *job = KManageSieve::SieveJob::list(u);
            connect(job, &KManageSieve::SieveJob::gotList, this, &CustomManageSieveWidget::slotGotList);
            mJobs.insert(job, last);
            mUrls.insert(last, u);
            last->startAnimation();
        }
        last->setText(0, serverName);
        noImapFound = false;
    }
    return noImapFound;
}
