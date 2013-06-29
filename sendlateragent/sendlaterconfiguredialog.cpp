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
#include "sendlaterdialog.h"
#include "sendlaterinfo.h"

#include "kdepim-version.h"

#include <KConfigGroup>
#include <KLocale>
#include <KHelpMenu>
#include <KMenu>
#include <KAboutData>
#include <KMessageBox>

#include <QPointer>

static QString sendLaterItemPattern = QLatin1String( "SendLaterItem \\d+" );


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
                ki18n( "Copyright (C) 2013 Laurent Montel" ) );

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
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void SendLaterConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterConfigureDialog" );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
}

SendLaterItem::SendLaterItem(QTreeWidget *parent )
    : QTreeWidgetItem(parent),
      mInfo(0)
{
}

SendLaterItem::~SendLaterItem()
{
    delete mInfo;
}

void SendLaterItem::setInfo(SendLaterInfo *info)
{
    mInfo = info;
}

SendLaterInfo* SendLaterItem::info() const
{
    return mInfo;
}


SendLaterWidget::SendLaterWidget( QWidget *parent )
    : QWidget( parent ),
      mChanged(false)
{
    mWidget = new Ui::SendLaterWidget;
    mWidget->setupUi( this );
    QStringList headers;
    headers << i18n("Subject")<<i18n("Date")<<i18n("Recursive")<<i18n("Message Id");
    mWidget->treeWidget->setHeaderLabels(headers);
    mWidget->treeWidget->setSortingEnabled(true);
    mWidget->treeWidget->setRootIsDecorated(false);
    mWidget->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mWidget->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mWidget->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    connect(mWidget->removeItem, SIGNAL(clicked(bool)), SLOT(slotRemoveItem()));
    connect(mWidget->modifyItem, SIGNAL(clicked(bool)), SLOT(slotModifyItem()));
    connect(mWidget->treeWidget, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()));
    connect(mWidget->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotModifyItem()));
    load();
    updateButtons();
}

SendLaterWidget::~SendLaterWidget()
{
    delete mWidget;
}

void SendLaterWidget::customContextMenuRequested(const QPoint &)
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        KMenu menu;
        if ( listItems.count() == 1) {
            menu.addAction(i18n("Send now"), this, SLOT(slotSendNow()));
        }
        menu.addSeparator();
        menu.addAction(i18n("Delete"), this, SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void SendLaterWidget::slotSendNow()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count() == 1) {
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(listItems.first());
        Q_EMIT sendNow(mailItem->info()->itemId());
    }
}

void SendLaterWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mWidget->treeWidget->header()->restoreState(data);
}

void SendLaterWidget::saveTreeWidgetHeader(KConfigGroup &group)
{
    group.writeEntry( "HeaderState", mWidget->treeWidget->header()->saveState() );
}

void SendLaterWidget::updateButtons()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.isEmpty()) {
        mWidget->removeItem->setEnabled(false);
        mWidget->modifyItem->setEnabled(false);
    } else if (listItems.count() == 1) {
        mWidget->removeItem->setEnabled(true);
        mWidget->modifyItem->setEnabled(true);
    } else {
        mWidget->removeItem->setEnabled(true);
        mWidget->modifyItem->setEnabled(false);
    }
}

void SendLaterWidget::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    const QStringList filterGroups = config->groupList().filter( QRegExp( sendLaterItemPattern ) );
    const int numberOfItem = filterGroups.count();
    for (int i = 0 ; i < numberOfItem; ++i) {
        KConfigGroup group = config->group(filterGroups.at(i));
        SendLaterInfo *info = new SendLaterInfo(group);
        createOrUpdateItem(info);
    }
}

void SendLaterWidget::createOrUpdateItem(SendLaterInfo *info, SendLaterItem *item)
{
    if (!item) {
        item = new SendLaterItem(mWidget->treeWidget);
    }
    item->setCheckState(Recursive, info->isRecursive() ? Qt::Checked : Qt::Unchecked);
    item->setText(MessageId, QString::number(info->itemId()));
    item->setText(Date, info->dateTime().toString());
    //TODO
    item->setInfo(info);
}

void SendLaterWidget::save()
{
    if (!mChanged)
        return;
    KSharedConfig::Ptr config = KGlobal::config();

    // first, delete all filter groups:
    const QStringList filterGroups =config->groupList().filter( QRegExp( sendLaterItemPattern ) );

    foreach ( const QString &group, filterGroups ) {
        config->deleteGroup( group );
    }

    const int numberOfItem(mWidget->treeWidget->topLevelItemCount());
    for (int i = 0; i < numberOfItem; ++i) {
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(mWidget->treeWidget->topLevelItem(i));
        if (mailItem->info()) {
            KConfigGroup group = config->group(QString::fromLatin1("SendLaterItem %1").arg(mailItem->info()->itemId()));
            mailItem->info()->writeConfig(group);
        }
    }
    config->sync();
    config->reparseConfiguration();
}

void SendLaterWidget::slotRemoveItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (KMessageBox::warningYesNo(this,i18n("Do you want to delete selected items? Do you want to continue?"),i18n("Remove items"))== KMessageBox::No)
        return;

    Q_FOREACH(QTreeWidgetItem *item,listItems) {
        delete item;
    }
    mChanged = true;
    updateButtons();
}

void SendLaterWidget::slotModifyItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count()==1) {
        QTreeWidgetItem *item = listItems.first();
        if (!item)
            return;
        SendLaterItem *mailItem = static_cast<SendLaterItem *>(item);

        QPointer<SendLaterDialog> dialog = new SendLaterDialog(mailItem->info(), this);
        if (dialog->exec()) {
            SendLaterInfo *info = dialog->info();
            createOrUpdateItem(info, mailItem);
            mChanged = true;
        }
        delete dialog;
    }
}

void SendLaterWidget::needToReload()
{
    //TODO
    mWidget->treeWidget->clear();

}

#include "sendlaterconfiguredialog.moc"
