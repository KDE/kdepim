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

#include "manageaccountwidget.h"

#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agenttypedialog.h>

#include <Akonadi/AgentFilterProxyModel>
#include <Akonadi/AgentManager>
#include "ui_manageaccountwidget.h"

#include <KMessageBox>
#include <KLocalizedString>
#include <KWindowSystem>

using namespace PimCommon;

ManageAccountWidget::ManageAccountWidget(QWidget *parent)
    : QWidget(parent)
{
    mWidget = new Ui::ManageAccountWidget;
    mWidget->setupUi( this );
    connect( mWidget->mAddAccountButton, SIGNAL(clicked()),
             this, SLOT(slotAddAccount()) );

    connect( mWidget->mModifyAccountButton, SIGNAL(clicked()),
             this, SLOT(slotModifySelectedAccount()) );

    connect( mWidget->mRemoveAccountButton, SIGNAL(clicked()),
             this, SLOT(slotRemoveSelectedAccount()) );
    connect( mWidget->mRestartAccountButton, SIGNAL(clicked()),
             this, SLOT(slotRestartSelectedAccount()) );

}

ManageAccountWidget::~ManageAccountWidget()
{
    delete mWidget;
}

void ManageAccountWidget::setSpecialCollectionIdentifier(const QString &identifier)
{
    mSpecialCollectionIdentifier = identifier;
}

void ManageAccountWidget::slotAddAccount()
{
    Akonadi::AgentTypeDialog dlg( this );

    Akonadi::AgentFilterProxyModel* filter = dlg.agentFilterProxyModel();
    filter->addMimeTypeFilter( mMimeTypeFilter );
    filter->addCapabilityFilter( mCapabilityFilter );
    Q_FOREACH(const QString &capa, mExcludeCapabilities) {
        filter->excludeCapabilities( capa );
    }
    if ( dlg.exec() ) {
        const Akonadi::AgentType agentType = dlg.agentType();

        if ( agentType.isValid() ) {

            Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( agentType, this );
            job->configure( this );
            job->start();
        }
    }
}

QStringList ManageAccountWidget::excludeCapabilities() const
{
    return mExcludeCapabilities;
}

void ManageAccountWidget::setExcludeCapabilities(const QStringList &excludeCapabilities)
{
    mExcludeCapabilities = excludeCapabilities;
}

QString ManageAccountWidget::capabilityFilter() const
{
    return mCapabilityFilter;
}

void ManageAccountWidget::setCapabilityFilter(const QString &capabilityFilter)
{
    mCapabilityFilter = capabilityFilter;
}

QString ManageAccountWidget::mimeTypeFilter() const
{
    return mMimeTypeFilter;
}

void ManageAccountWidget::setMimeTypeFilter(const QString &mimeTypeFilter)
{
    mMimeTypeFilter = mimeTypeFilter;
}


void ManageAccountWidget::slotModifySelectedAccount()
{
    Akonadi::AgentInstance instance = mWidget->mAccountList->currentAgentInstance();
    if ( instance.isValid() ) {
        KWindowSystem::allowExternalProcessWindowActivation();
        instance.configure( this );
    }
}

void ManageAccountWidget::slotRestartSelectedAccount()
{
    const Akonadi::AgentInstance instance =  mWidget->mAccountList->currentAgentInstance();
    if ( instance.isValid() ) {
        instance.restart();
    }
}

void ManageAccountWidget::slotRemoveSelectedAccount()
{
    const Akonadi::AgentInstance instance =  mWidget->mAccountList->currentAgentInstance();

    const int rc = KMessageBox::questionYesNo( this,
                                         i18n("Do you want to remove account '%1'?", instance.name()),
                                         i18n("Remove account?"));
    if ( rc == KMessageBox::No ) {
        return;
    }

    if ( instance.isValid() )
        Akonadi::AgentManager::self()->removeInstance( instance );

    slotAccountSelected( mWidget->mAccountList->currentAgentInstance() );

}

void ManageAccountWidget::slotAccountSelected(const Akonadi::AgentInstance& current)
{
    if (current.isValid()) {
        mWidget->mModifyAccountButton->setEnabled( !current.type().capabilities().contains( QLatin1String( "NoConfig" ) ) );
        mWidget->mRemoveAccountButton->setEnabled( mSpecialCollectionIdentifier != current.identifier() );
        // Restarting an agent is not possible if it's in Running status... (see AgentProcessInstance::restartWhenIdle)
        mWidget->mRestartAccountButton->setEnabled( ( current.status() != 1 ) );
    } else {
        mWidget->mModifyAccountButton->setEnabled( false );
        mWidget->mRemoveAccountButton->setEnabled( false );
        mWidget->mRestartAccountButton->setEnabled( false );
    }
}
