/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "archivemaildialog.h"
#include "addarchivemaildialog.h"
#include "archivemailagentutil.h"

#include "kdepim-version.h"

#include <mailcommon/util/mailutil.h>

#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KMenu>
#include <KRun>
#include <KAction>
#include <KHelpMenu>
#include <KAboutData>

#include <QHBoxLayout>


static QString archiveMailCollectionPattern = QLatin1String( "ArchiveMailCollection \\d+" );

ArchiveMailDialog::ArchiveMailDialog(QWidget *parent)
    :KDialog(parent)
{
    setCaption( i18n( "Configure Archive Mail Agent" ) );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Help | Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mWidget = new ArchiveMailWidget(this);
    connect(mWidget, SIGNAL(archiveNow(ArchiveMailInfo*)), this, SIGNAL(archiveNow(ArchiveMailInfo*)));
    mainLayout->addWidget(mWidget);
    setMainWidget( mainWidget );
    connect(this, SIGNAL(okClicked()), SLOT(slotSave()));
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

    mAboutData->setProgramIconName( QLatin1String("kmail") );
    mAboutData->setTranslator( ki18nc( "NAME OF TRANSLATORS", "Your names" ),
                             ki18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );


    KHelpMenu *helpMenu = new KHelpMenu(this, mAboutData, true);
    //Initialize menu
    KMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(KIcon(QLatin1String("kmail")));
    setButtonMenu( Help, menu );
}

ArchiveMailDialog::~ArchiveMailDialog()
{
    writeConfig();
    delete mAboutData;
}

void ArchiveMailDialog::slotNeedReloadConfig()
{
    mWidget->needReloadConfig();
}

static const char *myConfigGroupName = "ArchiveMailDialog";

void ArchiveMailDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }

    mWidget->restoreTreeWidgetHeader(group.readEntry("HeaderState",QByteArray()));
}

void ArchiveMailDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), myConfigGroupName );
    group.writeEntry( "Size", size() );
    mWidget->saveTreeWidgetHeader(group);
    group.sync();
}

void ArchiveMailDialog::slotSave()
{
    mWidget->save();
}


ArchiveMailItem::ArchiveMailItem(QTreeWidget *parent )
    : QTreeWidgetItem(parent),mInfo(0)
{
}

ArchiveMailItem::~ArchiveMailItem()
{
    delete mInfo;
}

void ArchiveMailItem::setInfo(ArchiveMailInfo *info)
{
    mInfo = info;
}

ArchiveMailInfo* ArchiveMailItem::info() const
{
    return mInfo;
}


ArchiveMailWidget::ArchiveMailWidget( QWidget *parent )
    : QWidget( parent ),
      mChanged(false)
{
    mWidget = new Ui::ArchiveMailWidget;
    mWidget->setupUi( this );
    QStringList headers;
    headers<<i18n("Name")<<i18n("Last archive")<<i18n("Next archive in")<<i18n("Storage directory");
    mWidget->treeWidget->setHeaderLabels(headers);
    mWidget->treeWidget->setSortingEnabled(true);
    mWidget->treeWidget->setRootIsDecorated(false);
    mWidget->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mWidget->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mWidget->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    load();
    connect(mWidget->removeItem, SIGNAL(clicked(bool)), SLOT(slotRemoveItem()));
    connect(mWidget->modifyItem, SIGNAL(clicked(bool)), SLOT(slotModifyItem()));
    connect(mWidget->addItem, SIGNAL(clicked(bool)), SLOT(slotAddItem()));
    connect(mWidget->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    connect(mWidget->treeWidget, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()));
    connect(mWidget->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotModifyItem()));
    updateButtons();
}

ArchiveMailWidget::~ArchiveMailWidget()
{
    delete mWidget;
}

void ArchiveMailWidget::customContextMenuRequested(const QPoint &)
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    KMenu menu;
    menu.addAction(i18n("Add..."),this,SLOT(slotAddItem()));
    if ( !listItems.isEmpty() ) {
        if ( listItems.count() == 1) {
            menu.addAction(i18n("Open Containing Folder..."), this, SLOT(slotOpenFolder()));
            menu.addSeparator();
            menu.addAction(i18n("Archive now"), this, SLOT(slotArchiveNow()));
        }
        menu.addSeparator();
        menu.addAction(KIcon(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(slotRemoveItem()));
    }
    menu.exec(QCursor::pos());
}

void ArchiveMailWidget::restoreTreeWidgetHeader(const QByteArray &data)
{
    mWidget->treeWidget->header()->restoreState(data);
}

void ArchiveMailWidget::saveTreeWidgetHeader(KConfigGroup& group)
{
    group.writeEntry( "HeaderState", mWidget->treeWidget->header()->saveState() );
}

void ArchiveMailWidget::updateButtons()
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

void ArchiveMailWidget::needReloadConfig()
{
    //TODO add messagebox which informs that we save settings here.
    mWidget->treeWidget->clear();
    load();
}

void ArchiveMailWidget::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    const QStringList collectionList = config->groupList().filter( QRegExp( archiveMailCollectionPattern ) );
    const int numberOfCollection = collectionList.count();
    for (int i = 0 ; i < numberOfCollection; ++i) {
        KConfigGroup group = config->group(collectionList.at(i));
        ArchiveMailInfo *info = new ArchiveMailInfo(group);
        createOrUpdateItem(info);
    }
}

void ArchiveMailWidget::createOrUpdateItem(ArchiveMailInfo *info, ArchiveMailItem *item)
{
    if (!item) {
        item = new ArchiveMailItem(mWidget->treeWidget);
    }
    item->setText(ArchiveMailWidget::Name,i18n("Folder: %1",MailCommon::Util::fullCollectionPath(Akonadi::Collection(info->saveCollectionId()))));
    item->setCheckState(ArchiveMailWidget::Name, info->isEnabled() ? Qt::Checked : Qt::Unchecked);
    item->setText(ArchiveMailWidget::StorageDirectory, info->url().toLocalFile());
    if (info->lastDateSaved().isValid()) {
        item->setText(ArchiveMailWidget::LastArchiveDate,KGlobal::locale()->formatDate(info->lastDateSaved()));
        updateDiffDate(item, info);
    } else {
        item->setBackgroundColor(ArchiveMailWidget::NextArchive,Qt::green);
    }
    item->setInfo(info);
}

void ArchiveMailWidget::updateDiffDate(ArchiveMailItem *item, ArchiveMailInfo *info)
{
    const QDate diffDate = ArchiveMailAgentUtil::diffDate(info);
    const int diff = QDate::currentDate().daysTo(diffDate);
    item->setText(ArchiveMailWidget::NextArchive,i18np("Tomorrow", "%1 days",diff));
    if (diff<0) {
        if (info->isEnabled())
            item->setBackgroundColor(ArchiveMailWidget::NextArchive,Qt::red);
        else
            item->setBackgroundColor(ArchiveMailWidget::NextArchive,Qt::lightGray);
    } else {
        item->setToolTip(ArchiveMailWidget::NextArchive,i18n("Archive will be done %1",KGlobal::locale()->formatDate(diffDate)));
    }
}

void ArchiveMailWidget::save()
{
    if (!mChanged)
        return;
    KSharedConfig::Ptr config = KGlobal::config();

    // first, delete all filter groups:
    const QStringList filterGroups =config->groupList().filter( QRegExp( archiveMailCollectionPattern ) );

    foreach ( const QString &group, filterGroups ) {
        config->deleteGroup( group );
    }

    const int numberOfItem(mWidget->treeWidget->topLevelItemCount());
    for (int i = 0; i < numberOfItem; ++i) {
        ArchiveMailItem *mailItem = static_cast<ArchiveMailItem *>(mWidget->treeWidget->topLevelItem(i));
        if (mailItem->info()) {
            KConfigGroup group = config->group( ArchiveMailAgentUtil::archivePattern.arg(mailItem->info()->saveCollectionId()));
            mailItem->info()->writeConfig(group);
        }
    }
    config->sync();
    config->reparseConfiguration();
}

void ArchiveMailWidget::slotRemoveItem()
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

void ArchiveMailWidget::slotModifyItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count()==1) {
        QTreeWidgetItem *item = listItems.at(0);
        if (!item)
            return;
        ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
        QPointer<AddArchiveMailDialog> dialog = new AddArchiveMailDialog(archiveItem->info(), this);
        if ( dialog->exec() ) {
            ArchiveMailInfo *info = dialog->info();
            createOrUpdateItem(info,archiveItem);
            mChanged = true;
        }
        delete dialog;
    }
}

void ArchiveMailWidget::slotAddItem()
{
    QPointer<AddArchiveMailDialog> dialog = new AddArchiveMailDialog(0,this);
    if ( dialog->exec() ) {
        ArchiveMailInfo *info = dialog->info();
        if (verifyExistingArchive(info)) {
            KMessageBox::error(this,i18n("Cannot add a second archive for this folder. Modify the existing one instead."),i18n("Add Archive Mail"));
            delete info;
        } else {
            createOrUpdateItem(info);
            updateButtons();
            mChanged = true;
        }
    }
    delete dialog;
}

bool ArchiveMailWidget::verifyExistingArchive(ArchiveMailInfo *info) const
{
    const int numberOfItem(mWidget->treeWidget->topLevelItemCount());
    for (int i = 0; i < numberOfItem; ++i) {
        ArchiveMailItem *mailItem = static_cast<ArchiveMailItem *>(mWidget->treeWidget->topLevelItem(i));
        ArchiveMailInfo *archiveItemInfo = mailItem->info();
        if (archiveItemInfo) {
            if (info->saveCollectionId() == archiveItemInfo->saveCollectionId()) {
                return true;
            }
        }
    }
    return false;
}

void ArchiveMailWidget::slotOpenFolder()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count()==1) {
        QTreeWidgetItem *item = listItems.first();
        if (!item)
            return;
        ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
        ArchiveMailInfo *archiveItemInfo = archiveItem->info();
        if (archiveItemInfo) {
            const KUrl url = archiveItemInfo->url();
            KRun *runner = new KRun( url, this ); // will delete itself
            runner->setRunExecutables( false );
        }
    }
}

void ArchiveMailWidget::slotArchiveNow()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (listItems.count()==1) {
        QTreeWidgetItem *item = listItems.first();
        if (!item)
            return;
        ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
        ArchiveMailInfo *archiveItemInfo = archiveItem->info();
        save();
        if (archiveItemInfo) {
            Q_EMIT archiveNow(archiveItemInfo);
        }
    }
}

void ArchiveMailWidget::slotItemChanged(QTreeWidgetItem *item,int col)
{
    if (item) {
        ArchiveMailItem *archiveItem = static_cast<ArchiveMailItem*>(item);
        if (archiveItem->info()) {
            if (col == ArchiveMailWidget::Name) {
                archiveItem->info()->setEnabled(archiveItem->checkState(ArchiveMailWidget::Name) == Qt::Checked);
                mChanged = true;
            } else if (col == ArchiveMailWidget::NextArchive) {
                updateDiffDate(archiveItem, archiveItem->info());
            }
        }
    }
}

