/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditormanagesievewidget.h"
#include "widgets/sievetreewidgetitem.h"

#include <KLocalizedString>

using namespace KSieveUi;

SieveEditorManageSieveWidget::SieveEditorManageSieveWidget(QWidget *parent)
    : KSieveUi::ManageSieveWidget(parent)
{

}

SieveEditorManageSieveWidget::~SieveEditorManageSieveWidget()
{

}

bool SieveEditorManageSieveWidget::refreshList()
{
    bool noImapFound = true;
    SieveTreeWidgetItem *last = 0;
#if 0
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &type, lst ) {
        if ( type.status() == Akonadi::AgentInstance::Broken )
            continue;

        last = new SieveTreeWidgetItem( treeView(), last );
        last->setText( 0, type.name() );
        last->setIcon( 0, SmallIcon( QLatin1String("network-server") ) );

        const KUrl u = KSieveUi::Util::findSieveUrlForAccount( type.identifier() );
        if ( u.isEmpty() ) {
            QTreeWidgetItem *item = new QTreeWidgetItem( last );
            item->setText( 0, i18n( "No Sieve URL configured" ) );
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
            treeView()->expandItem( last );
        } else {
            KManageSieve::SieveJob * job = KManageSieve::SieveJob::list( u );
            connect( job, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
                     this, SLOT(slotGotList(KManageSieve::SieveJob*,bool,QStringList,QString)) );
            mJobs.insert( job, last );
            mUrls.insert( last, u );
            last->startAnimation();
        }
        noImapFound = false;
    }
#endif
    return noImapFound;
}

QTreeWidgetItem *SieveEditorManageSieveWidget::createNewItem(QTreeWidgetItem *parent)
{
    return new QTreeWidgetItem(parent);
}
