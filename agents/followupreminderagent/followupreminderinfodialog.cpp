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

#include "followupreminderinfodialog.h"
#include "followupreminderinfowidget.h"

#include "kdepim-version.h"

#include <KLocalizedString>
#include <KAboutData>
#include <KHelpMenu>
#include <KMenu>

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

FollowUpReminderInfoDialog::FollowUpReminderInfoDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Help|Ok|Cancel );

    QWidget *mainWidget = new QWidget( this );
    setMainWidget(mainWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));

    mWidget = new FollowUpReminderInfoWidget;
    mWidget->setObjectName(QLatin1String("FollowUpReminderInfoWidget"));
    mainLayout->addWidget(mWidget);

    readConfig();
    mAboutData = new KAboutData(
                QByteArray( "followupreminderagent" ),
                QByteArray(),
                ki18n( "Follow Up Reminder Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Follow Up Mail." ),
                KAboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2014 Laurent Montel" ) );

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

FollowUpReminderInfoDialog::~FollowUpReminderInfoDialog()
{
    writeConfig();
}

void FollowUpReminderInfoDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "FollowUpReminderInfoDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void FollowUpReminderInfoDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "FollowUpReminderInfoDialog" );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
}

void FollowUpReminderInfoDialog::slotSave()
{
    mWidget->save();
}

void FollowUpReminderInfoDialog::load()
{
    mWidget->load();
}

void FollowUpReminderInfoDialog::setInfo(const QList<FollowUpReminder::FollowUpReminderInfo *> &info)
{
    mWidget->setInfo(info);
}

QList<qint32> FollowUpReminderInfoDialog::listRemoveId() const
{
    return mWidget->listRemoveId();
}

