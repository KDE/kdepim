/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "notesagentsettingsdialog.h"

#include "noteshared/config/notenetworkconfig.h"

#include "kdepim-version.h"
#include <KMenu>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KIcon>
#include <KAboutData>
#include <KNotifyConfigWidget>

#include <QHBoxLayout>
#include <QTabWidget>

NotesAgentSettingsDialog::NotesAgentSettingsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure Notes Agent" ) );
    setWindowIcon( KIcon( QLatin1String("knotes") ) );
    setButtons( Help | Ok|Cancel );
    setDefaultButton( Ok );
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );

    QTabWidget *tab = new QTabWidget;
    mainLayout->addWidget(tab);

    mNotify = new KNotifyConfigWidget(this);
    mNotify->setApplication(QLatin1String("akonadi_notes_agent"));
    tab->addTab(mNotify, i18n("Notify"));

    mNetworkConfig = new NoteShared::NoteNetworkConfigWidget(this);
    tab->addTab(mNetworkConfig, i18n("Network"));
    mNetworkConfig->load();


    setMainWidget(mainWidget);
    readConfig();
    mAboutData = new KAboutData(
                QByteArray( "notesagent" ),
                QByteArray(),
                ki18n( "Notes Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Notes Agent." ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2013, 2014 Laurent Montel" ) );

    mAboutData->addAuthor( ki18n( "Laurent Montel" ),
                         ki18n( "Maintainer" ), "montel@kde.org" );

    mAboutData->setProgramIconName( QLatin1String("knotes") );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );


    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    //Initialize menu
    KMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(KIcon(QLatin1String("knotes")));
    setButtonMenu( Help, menu );
}

NotesAgentSettingsDialog::~NotesAgentSettingsDialog()
{
    writeConfig();
    delete mAboutData;
}

static const char *myConfigGroupName = "NotesAgentSettingsDialog";
void NotesAgentSettingsDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NotesAgentSettingsDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );
    group.writeEntry( "Size", size() );
    group.sync();
}

void NotesAgentSettingsDialog::slotOkClicked()
{
    mNotify->save();
    mNetworkConfig->save();
}
