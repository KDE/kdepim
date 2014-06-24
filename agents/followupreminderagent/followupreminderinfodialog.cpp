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

#include "kdepim-version.h"

#include <KLocalizedString>
#include <K4AboutData>
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
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));

    mWidget = new FollowUpReminderInfoWidget;
    mainLayout->addWidget(mWidget);

    readConfig();
    mAboutData = new K4AboutData(
                QByteArray( "followupreminderagent" ),
                QByteArray(),
                ki18n( "Follow Up Reminder Agent" ),
                QByteArray( KDEPIM_VERSION ),
                ki18n( "Follow Up Mail." ),
                K4AboutData::License_GPL_V2,
                ki18n( "Copyright (C) 2014 Laurent Montel" ) );

    mAboutData->addAuthor( ki18n( "Laurent Montel" ),
                         ki18n( "Maintainer" ), "montel@kde.org" );

    mAboutData->setProgramIconName( QLatin1String("kmail") );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );

#if 0 //QT5
    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    //Initialize menu
    KMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QLatin1String("kmail")));
    setButtonMenu( Help, menu );
#endif
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


FollowUpReminderInfoWidget::FollowUpReminderInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    mTreeWidget = new QTreeWidget;
    //TODO
    QStringList headers;
    headers << i18n("To")
            << i18n("Subject")
            << i18n("Message Id");

    mTreeWidget->setHeaderLabels(headers);
    mTreeWidget->setSortingEnabled(true);
    mTreeWidget->setRootIsDecorated(false);
    mTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    hbox->addWidget(mTreeWidget);
    setLayout(hbox);
}

FollowUpReminderInfoWidget::~FollowUpReminderInfoWidget()
{
}

void FollowUpReminderInfoWidget::customContextMenuRequested(const QPoint &pos)
{
    const QList<QTreeWidgetItem *> listItems = mTreeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        QMenu menu;
        menu.addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void FollowUpReminderInfoWidget::slotRemoveItem()
{
    //TODO
}

void FollowUpReminderInfoWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mTreeWidget->header()->restoreState(data);
}

void FollowUpReminderInfoWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mTreeWidget->header()->saveState() );
}

