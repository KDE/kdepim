/*
    This file is part of Akregator2.
    Copyright (c) 2012 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "akregator2_config_feedlists.h"
#include "akregator2config.h"

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentFilterProxyModel>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentTypeWidget>

#include "ui_settings_feedlists.h"

#include <KAboutData>
#include <KConfigDialogManager>
#include <KDialog>
#include <KGenericFactory>
#include <KLocalizedString>
#include <KMessageBox>

#include <QTextDocument>

#include <kdemacros.h>


#include <QVBoxLayout>

K_PLUGIN_FACTORY(KCMAkregator2FeedListsConfigFactory, registerPlugin<KCMAkregator2FeedListsConfig>();)
K_EXPORT_PLUGIN(KCMAkregator2FeedListsConfigFactory( "kcmakrfeedlistsconfig" ))

AddFeedListDialog::AddFeedListDialog( QWidget *parent )
    : KDialog( parent )
    , m_agentTypeWidget( 0 )
{
    setWindowTitle( i18n("Create New Feed List") );
    resize( 480, 340 );
    setButtons( KDialog::Ok|KDialog::Cancel );
    m_agentTypeWidget = new Akonadi::AgentTypeWidget;
    m_agentTypeWidget->agentFilterProxyModel()->addCapabilityFilter( QLatin1String("RssResource") );
    setMainWidget( m_agentTypeWidget );
    connect( this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()) );
}

void AddFeedListDialog::slotOkClicked()
{
    const Akonadi::AgentType type = m_agentTypeWidget->currentAgentType();
    if ( type.isValid() )
        emit agentTypeSelected( type );

    accept();
}

KCMAkregator2FeedListsConfig::KCMAkregator2FeedListsConfig( QWidget* parent, const QVariantList& args )
    : KCModule( KCMAkregator2FeedListsConfigFactory::componentData(), parent, args )
    , m_widget( new QWidget )
    , m_ui( new Akregator2::Ui::SettingsFeedLists )
{  
    m_ui->setupUi( m_widget );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );

    KAboutData *about = new KAboutData( I18N_NOOP( "kcmakrfeedlistsconfig" ), 0,
                                        ki18n( "Configure Feed Lists" ),
                                        0, KLocalizedString(), KAboutData::License_GPL,
                                        ki18n( "(c), 2012 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );
    m_ui->agentInstanceWidget->agentFilterProxyModel()->addCapabilityFilter( QLatin1String("RssResource") );
    connect( m_ui->agentInstanceWidget, SIGNAL(doubleClicked(Akonadi::AgentInstance)), this, SLOT(agentDoubleClicked(Akonadi::AgentInstance)) );
    connect( m_ui->agentInstanceWidget, SIGNAL(currentChanged(Akonadi::AgentInstance,Akonadi::AgentInstance)), this, SLOT(currentAgentChanged(Akonadi::AgentInstance,Akonadi::AgentInstance)) );
    connect( m_ui->addButton, SIGNAL(clicked()), this, SLOT(add()) );
    connect( m_ui->modifyButton, SIGNAL(clicked()), this, SLOT(modify()) );
    connect( m_ui->removeButton, SIGNAL(clicked()), this, SLOT(remove()) );
    connect( m_ui->restartButton, SIGNAL(clicked()), this, SLOT(restart()) );

    addConfig( Akregator2::Settings::self(), m_widget );
}

KCMAkregator2FeedListsConfig::~KCMAkregator2FeedListsConfig()
{
    delete m_ui;
}

void KCMAkregator2FeedListsConfig::currentAgentChanged( const Akonadi::AgentInstance &current, const Akonadi::AgentInstance& previous )
{
    Q_UNUSED( previous )
    const bool enable = current.isValid();
    m_ui->modifyButton->setEnabled( enable );
    m_ui->removeButton->setEnabled( enable );
    m_ui->restartButton->setEnabled( enable );
}

void KCMAkregator2FeedListsConfig::agentDoubleClicked( const Akonadi::AgentInstance& )
{
    modify();
}

void KCMAkregator2FeedListsConfig::add()
{
    if ( m_addFeedListDialog )
        return;
    m_addFeedListDialog = new AddFeedListDialog( this );
    m_addFeedListDialog->setAttribute( Qt::WA_DeleteOnClose );
    connect( m_addFeedListDialog, SIGNAL(agentTypeSelected(Akonadi::AgentType)), this, SLOT(agentTypeSelected(Akonadi::AgentType)) );
    m_addFeedListDialog->show();
}

void KCMAkregator2FeedListsConfig::agentTypeSelected( const Akonadi::AgentType& type )
{
    Q_ASSERT( type.isValid() );
    Akonadi::AgentInstanceCreateJob* job = new Akonadi::AgentInstanceCreateJob( type );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(agentCreated(KJob*)) );
    job->start();
}

void KCMAkregator2FeedListsConfig::agentCreated( KJob* job )
{
    if ( job->error() == KJob::NoError ) {
        Akonadi::AgentInstanceCreateJob* createJob = qobject_cast<Akonadi::AgentInstanceCreateJob*>( job );
        Q_ASSERT( createJob );
        Akonadi::AgentInstance instance = createJob->instance();
        instance.configure( this );
        instance.synchronizeCollectionTree();
    }
}

void KCMAkregator2FeedListsConfig::modify()
{
    Akonadi::AgentInstance instance = m_ui->agentInstanceWidget->currentAgentInstance();
    if ( instance.isValid() )
        instance.configure( this );
}

void KCMAkregator2FeedListsConfig::remove()
{
    Akonadi::AgentInstance instance = m_ui->agentInstanceWidget->currentAgentInstance();
    if ( !instance.isValid() )
        return;
    if ( KMessageBox::warningYesNo( this,
                                    i18n("<qt>Do you really want to delete the feed list <strong>%1</strong>? All contained feeds will be deleted from disk!</qt>", Qt::escape( instance.name() ) ),
                                    i18n("Delete Feed List"),
                                    KStandardGuiItem::del(),
                                    KStandardGuiItem::cancel() ) == KMessageBox::Yes )
        Akonadi::AgentManager::self()->removeInstance( instance );
}

void KCMAkregator2FeedListsConfig::restart()
{
    Akonadi::AgentInstance instance = m_ui->agentInstanceWidget->currentAgentInstance();
    if ( instance.isValid() )
        instance.restart();
}

