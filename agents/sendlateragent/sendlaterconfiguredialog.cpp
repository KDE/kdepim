/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "sendlaterconfiguredialog.h"
#include "sendlaterconfigurewidget.h"
#include "kdepim-version.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KHelpMenu>
#include <KMenu>
#include <KAboutData>
#include <KMessageBox>

static QString sendLaterItemPattern = QLatin1String( "SendLaterItem \\d+" );

//#define DEBUG_MESSAGE_ID

SendLaterConfigureDialog::SendLaterConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Help|Ok|Cancel );

    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mWidget = new SendLaterWidget(this);
    mWidget->setObjectName(QLatin1String("sendlaterwidget"));
    connect(mWidget, SIGNAL(sendNow(Akonadi::Item::Id)), SIGNAL(sendNow(Akonadi::Item::Id)));
    mainLayout->addWidget(mWidget);
    setMainWidget( mainWidget );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));

    readConfig();
    mAboutData = new KAboutData(
                QByteArray( "sendlateragent" ),
                QByteArray(),
                ki18n( "Send Later Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Send emails later agent." ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2013, 2014 Laurent Montel" ) );

    mAboutData->addAuthor( ki18n( "Laurent Montel" ),
                         ki18n( "Maintainer" ), "montel@kde.org" );

    mAboutData->setProgramIconName( QLatin1String("kmail") );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );


    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    //Initialize menu
    KMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(KIcon(QLatin1String("kmail")));
    setButtonMenu( Help, menu );
}

SendLaterConfigureDialog::~SendLaterConfigureDialog()
{
    writeConfig();
    delete mAboutData;
}

QList<Akonadi::Item::Id> SendLaterConfigureDialog::messagesToRemove() const
{
    return mWidget->messagesToRemove();
}


void SendLaterConfigureDialog::slotSave()
{
    mWidget->save();
}

void SendLaterConfigureDialog::slotNeedToReloadConfig()
{
    mWidget->needToReload();
}

void SendLaterConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterConfigureDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void SendLaterConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterConfigureDialog" );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
}

