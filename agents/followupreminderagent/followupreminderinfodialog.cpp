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

#include "followupreminderinfodialog.h"
#include "followupreminderinfowidget.h"

#include "kdepim-version.h"

#include <KLocalizedString>
#include <KAboutData>
#include <KHelpMenu>
#include <QMenu>
#include <QIcon>

#include <QTreeWidget>
#include <QHBoxLayout>
#include <KSharedConfig>
#include <QHeaderView>

FollowUpReminderInfoDialog::FollowUpReminderInfoDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure") );
    setWindowIcon( QIcon::fromTheme( QLatin1String("kmail") ) );
    setButtons( Help|Ok|Cancel );

    QWidget *mainWidget = new QWidget( this );
    setMainWidget(mainWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));

    mWidget = new FollowUpReminderInfoWidget;
    mainLayout->addWidget(mWidget);

    readConfig();
    KAboutData aboutData = KAboutData(
                QLatin1String( "followupreminderagent" ),
                i18n( "Follow Up Reminder Agent" ),
                QLatin1String( KDEPIM_VERSION ),
                i18n( "Follow Up Mail." ),
                KAboutLicense::GPL_V2,
                i18n( "Copyright (C) 2014 Laurent Montel" ) );

    aboutData.addAuthor( i18n( "Laurent Montel" ),
                         i18n( "Maintainer" ), QLatin1String("montel@kde.org") );

    aboutData.setProgramIconName( QLatin1String("kmail") );
    aboutData.setTranslator( i18nc( "NAME OF TRANSLATORS", "Your names" ),
                             i18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );

    KHelpMenu *helpMenu = new KHelpMenu(this, aboutData, true);
    //Initialize menu
    QMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QLatin1String("kmail")));
    setButtonMenu( Help, menu );
}

FollowUpReminderInfoDialog::~FollowUpReminderInfoDialog()
{
    writeConfig();
}

void FollowUpReminderInfoDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "FollowUpReminderInfoDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void FollowUpReminderInfoDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "SendLaterConfigureDialog" );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
}
