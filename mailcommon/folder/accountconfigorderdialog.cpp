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

#include "accountconfigorderdialog.h"
#include "mailcommon/mailcommonsettings_base.h"
#include "pimcommon/util/pimutil.h"
#include "mailcommon/util/mailutil.h"

#include <KLocalizedString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QIcon>

#include <AkonadiCore/AgentInstance>
#include <AkonadiCore/AgentManager>

#include <KMime/KMimeMessage>

#include <QListWidget>
#include <QHBoxLayout>
#include <QCheckBox>

using namespace MailCommon;

struct InstanceStruct {
    QString name;
    QIcon icon;
};


AccountConfigOrderDialog::AccountConfigOrderDialog(QWidget *parent)
    :KDialog(parent)
{
    setCaption( i18n("Edit Accounts Order") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );

    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *vbox = new QVBoxLayout;
    page->setLayout(vbox);

    mEnableAccountOrder = new QCheckBox(i18n("Use custom order"));
    connect(mEnableAccountOrder, SIGNAL(clicked(bool)), this, SLOT(slotEnableAccountOrder(bool)));
    vbox->addWidget(mEnableAccountOrder);

    QHBoxLayout * vlay = new QHBoxLayout;
    vbox->addLayout(vlay);

    mListAccount = new QListWidget(this);
    mListAccount->setDragDropMode( QAbstractItemView::InternalMove );
    vlay->addWidget(mListAccount);

    QWidget* upDownBox = new QWidget( page );
    QVBoxLayout *upDownBoxVBoxLayout = new QVBoxLayout(upDownBox);
    upDownBoxVBoxLayout->setMargin(0);
    mUpButton = new QPushButton( upDownBox );
    upDownBoxVBoxLayout->addWidget(mUpButton);
    mUpButton->setIcon( QIcon::fromTheme(QLatin1String("go-up")) );
    mUpButton->setToolTip( i18nc( "Move selected account up.", "Up" ) );
    mUpButton->setEnabled( false ); // b/c no item is selected yet
    mUpButton->setFocusPolicy( Qt::StrongFocus );
    mUpButton->setAutoRepeat(true);

    mDownButton = new QPushButton( upDownBox );
    upDownBoxVBoxLayout->addWidget(mDownButton);
    mDownButton->setIcon( QIcon::fromTheme(QLatin1String("go-down")) );
    mDownButton->setToolTip( i18nc( "Move selected account down.", "Down" ) );
    mDownButton->setEnabled( false ); // b/c no item is selected yet
    mDownButton->setFocusPolicy( Qt::StrongFocus );
    mDownButton->setAutoRepeat(true);

    QWidget* spacer = new QWidget( upDownBox );
    upDownBoxVBoxLayout->addWidget(spacer);
    upDownBoxVBoxLayout->setStretchFactor( spacer, 100 );
    vlay->addWidget( upDownBox );


    connect( mUpButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()) );
    connect( mDownButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()) );
    connect( mListAccount, SIGNAL(itemSelectionChanged()), this, SLOT(slotEnableControls()));
    connect( mListAccount->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),SLOT(slotEnableControls()) );

    connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
    readConfig();
    init();
}

AccountConfigOrderDialog::~AccountConfigOrderDialog()
{
    writeConfig();
}

void AccountConfigOrderDialog::slotEnableAccountOrder(bool state)
{
    mListAccount->setEnabled(state);
    mUpButton->setEnabled(state);
    mDownButton->setEnabled(state);
    if (state)
        slotEnableControls();
}

void AccountConfigOrderDialog::slotMoveUp()
{
    if ( !mListAccount->currentItem() )
        return;
    const int pos = mListAccount->row( mListAccount->currentItem() );
    mListAccount->blockSignals( true );
    QListWidgetItem *item = mListAccount->takeItem( pos );
    // now selected item is at idx(idx-1), so
    // insert the other item at idx, ie. above(below).
    mListAccount->insertItem( pos -1,  item );
    mListAccount->blockSignals( false );
    mListAccount->setCurrentRow( pos - 1 );
}

void AccountConfigOrderDialog::slotMoveDown()
{
    if ( !mListAccount->currentItem() )
        return;
    const int pos = mListAccount->row( mListAccount->currentItem() );
    mListAccount->blockSignals( true );
    QListWidgetItem *item = mListAccount->takeItem( pos );
    // now selected item is at idx(idx-1), so
    // insert the other item at idx, ie. above(below).
    mListAccount->insertItem( pos +1 , item );
    mListAccount->blockSignals( false );
    mListAccount->setCurrentRow( pos + 1 );
}


void AccountConfigOrderDialog::slotEnableControls()
{
    QListWidgetItem *item = mListAccount->currentItem();

    mUpButton->setEnabled( item && mListAccount->currentRow()!=0 );
    mDownButton->setEnabled( item && mListAccount->currentRow()!=mListAccount->count()-1 );
}

void AccountConfigOrderDialog::init()
{
    const QStringList listOrderAccount = MailCommon::MailCommonSettings::self()->order();
    QStringList instanceList;

    QMap<QString, InstanceStruct> mapInstance;
    foreach ( const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances() ) {
        const QStringList capabilities( instance.type().capabilities() );
        if ( instance.type().mimeTypes().contains( KMime::Message::mimeType() ) ) {
            if ( capabilities.contains( QLatin1String("Resource") ) &&
                 !capabilities.contains( QLatin1String("Virtual") ) &&
                 !capabilities.contains( QLatin1String("MailTransport") ) )
            {
                const QString identifier = instance.identifier();
                if (!identifier.contains(POP3_RESOURCE_IDENTIFIER)) {
                    instanceList<<instance.identifier();
                    InstanceStruct instanceStruct;
                    instanceStruct.name = instance.name();
                    if (PimCommon::Util::isImapResource(identifier)) {
                        instanceStruct.name += QLatin1String(" (IMAP)");
                    } else if (identifier.startsWith(QLatin1String("akonadi_maildir_resource"))) {
                        instanceStruct.name += QLatin1String(" (Maildir)");
                    } else if (identifier.startsWith(QLatin1String("akonadi_mailbox_resource"))) {
                        instanceStruct.name += QLatin1String(" (Mailbox)");
                    } else if (identifier.startsWith(QLatin1String("akonadi_mixedmaildir_resource"))) {
                        instanceStruct.name += QLatin1String(" (Mixedmaildir)");
                    } else {
                        qDebug()<<" Unknown resource type "<<identifier;
                    }
                    instanceStruct.icon = instance.type().icon();
                    mapInstance.insert(instance.identifier(),instanceStruct);
                }
            }
        }
    }
    const int numberOfList(listOrderAccount.count());
    for (int i = 0; i< numberOfList;++i) {
        instanceList.removeOne(listOrderAccount.at(i));
    }

    QStringList finalList (listOrderAccount);
    finalList += instanceList;

    const int numberOfElement(finalList.count());
    for (int i = 0; i <numberOfElement;++i) {
        const QString identifier(finalList.at(i));
        if (mapInstance.contains(identifier)) {
            InstanceStruct tmp = mapInstance.value(identifier);
            QListWidgetItem * item = new QListWidgetItem(tmp.icon,tmp.name,mListAccount);
            item->setData(AccountConfigOrderDialog::IdentifierAccount, identifier);
            mListAccount->addItem( item );
        }
    }
    mEnableAccountOrder->setChecked(MailCommon::MailCommonSettings::self()->enableAccountOrder());
    slotEnableAccountOrder(MailCommon::MailCommonSettings::self()->enableAccountOrder());
}

void AccountConfigOrderDialog::slotOk()
{
    QStringList order;
    const int numberOfItems(mListAccount->count());
    for (int i = 0; i<numberOfItems; ++i) {
        order << mListAccount->item(i)->data(AccountConfigOrderDialog::IdentifierAccount).toString();
    }

    MailCommon::MailCommonSettings::self()->setOrder(order);
    MailCommon::MailCommonSettings::self()->setEnableAccountOrder(mEnableAccountOrder->isChecked());
    MailCommon::MailCommonSettings::self()->save();
    KDialog::accept();
}

void AccountConfigOrderDialog::readConfig()
{
    KConfigGroup accountConfigDialog( MailCommon::MailCommonSettings::self()->config(), "AccountConfigOrderDialog" );
    const QSize size = accountConfigDialog.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void AccountConfigOrderDialog::writeConfig()
{
    KConfigGroup accountConfigDialog( MailCommon::MailCommonSettings::self()->config(), "AccountConfigOrderDialog" );
    accountConfigDialog.writeEntry( "Size", size() );
    accountConfigDialog.sync();
}



