/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "kdepim-version.h"

#include <KConfigGroup>
#include <KLocale>
#include <KHelpMenu>
#include <KMenu>
#include <KAboutData>

SendLaterConfigureDialog::SendLaterConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure") );
    setWindowIcon( KIcon( "kmail" ) );
    setButtons( Help|Ok|Cancel );
    QWidget *w = new QWidget;
    setMainWidget(w);
    readConfig();
    mAboutData = new KAboutData(
                QByteArray( "archivemailagent" ),
                QByteArray(),
                ki18n( "Archive Mail Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Archive emails automatically." ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2012, 2013 Laurent Montel" ) );

    mAboutData->addAuthor( ki18n( "Laurent Montel" ),
                         ki18n( "Maintainer" ), "montel@kde.org" );

    mAboutData->setProgramIconName( "kmail" );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );


    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    setButtonMenu( Help, helpMenu->menu() );
}

SendLaterConfigureDialog::~SendLaterConfigureDialog()
{
    writeConfig();
    delete mAboutData;
}

void SendLaterConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterConfigureDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
}

void SendLaterConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterConfigureDialog" );
    group.writeEntry( "Size", size() );
}


#include "sendlaterconfiguredialog.moc"
