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
#include <KMessageBox>

SendLaterConfigureDialog::SendLaterConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure") );
    setWindowIcon( KIcon( "kmail" ) );
    setButtons( Help|Ok|Cancel );

    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mWidget = new SendLaterWidget(this);
    mainLayout->addWidget(mWidget);
    setMainWidget( mainWidget );
    connect(this,SIGNAL(okClicked()),SLOT(slotSave()));

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

void SendLaterConfigureDialog::slotSave()
{
    mWidget->save();
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

SendLaterWidget::SendLaterWidget( QWidget *parent )
    : QWidget( parent )
{
    mWidget = new Ui::SendLaterWidget;
    mWidget->setupUi( this );
    QStringList headers;
    headers << i18n("Subject")<<i18n("Date")<<i18n("Recursive");
    mWidget->treeWidget->setHeaderLabels(headers);
    mWidget->treeWidget->setSortingEnabled(true);
    mWidget->treeWidget->setRootIsDecorated(false);
    mWidget->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mWidget->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mWidget->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    load();
    connect(mWidget->removeItem,SIGNAL(clicked(bool)),SLOT(slotRemoveItem()));
    connect(mWidget->modifyItem,SIGNAL(clicked(bool)),SLOT(slotModifyItem()));
    connect(mWidget->treeWidget,SIGNAL(itemSelectionChanged()),SLOT(updateButtons()));
    connect(mWidget->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),SLOT(slotModifyItem()));
    updateButtons();
}

SendLaterWidget::~SendLaterWidget()
{
    delete mWidget;
}

void SendLaterWidget::customContextMenuRequested(const QPoint&)
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if ( !listItems.isEmpty() ) {
        KMenu menu;
        if ( listItems.count() == 1) {
            menu.addAction(i18n("Open Containing Folder..."),this,SLOT(slotOpenFolder()));
        }
        menu.addSeparator();
        menu.addAction(i18n("Delete"),this,SLOT(slotRemoveItem()));
        menu.exec(QCursor::pos());
    }
}

void SendLaterWidget::restoreTreeWidgetHeader(const QByteArray& data)
{
    mWidget->treeWidget->header()->restoreState(data);
}

void SendLaterWidget::saveTreeWidgetHeader(KConfigGroup& group)
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
    //TODO
}


void SendLaterWidget::save()
{
    //TODO
}

void SendLaterWidget::slotRemoveItem()
{
    const QList<QTreeWidgetItem *> listItems = mWidget->treeWidget->selectedItems();
    if (KMessageBox::warningYesNo(this,i18n("Do you want to delete selected items? Do you want to continue?"),i18n("Remove items"))== KMessageBox::No)
        return;

    Q_FOREACH(QTreeWidgetItem *item,listItems) {
        delete item;
    }
    updateButtons();
}

void SendLaterWidget::slotModifyItem()
{
    //TODO
}


#include "sendlaterconfiguredialog.moc"
