/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "recentaddressdialog.h"
#include "recentaddresses.h"
#include <kpimutils/email.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KLineEdit>
#include <KPushButton>
#include <KMessageBox>

#include <QCoreApplication>
#include <QLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QKeyEvent>

using namespace KPIM;

RecentAddressDialog::RecentAddressDialog( QWidget *parent )
    : KDialog( parent ),
      mDirty(false)
{
    setCaption( i18n( "Edit Recent Addresses" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *layout = new QVBoxLayout( page );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    mLineEdit = new KLineEdit(this);
    layout->addWidget(mLineEdit);

    mLineEdit->setTrapReturnKey(true);
    mLineEdit->installEventFilter(this);

    connect(mLineEdit,SIGNAL(textChanged(QString)),SLOT(slotTypedSomething(QString)));
    connect(mLineEdit,SIGNAL(returnPressed()),SLOT(slotAddItem()));


    QHBoxLayout* hboxLayout = new QHBoxLayout;

    QVBoxLayout* btnsLayout = new QVBoxLayout;
    btnsLayout->addStretch();
    mNewButton = new KPushButton(KIcon(QLatin1String("list-add")), i18n("&Add"), this);
    connect(mNewButton, SIGNAL(clicked()), SLOT(slotAddItem()));
    btnsLayout->insertWidget(0 ,mNewButton);

    mRemoveButton = new KPushButton(KIcon(QLatin1String("list-remove")), i18n("&Remove"), this);
    mRemoveButton->setEnabled(false);
    connect(mRemoveButton, SIGNAL(clicked()), SLOT(slotRemoveItem()));
    btnsLayout->insertWidget(1, mRemoveButton);


    mListView = new QListWidget(this);
    mListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mListView->setSortingEnabled(true);
    hboxLayout->addWidget(mListView);
    hboxLayout->addLayout(btnsLayout);
    layout->addLayout(hboxLayout);
    connect(mListView, SIGNAL(itemSelectionChanged()),
            SLOT(slotSelectionChanged()));
    // maybe supplied lineedit has some text already
    slotTypedSomething( mLineEdit->text() );
    readConfig();
}

RecentAddressDialog::~RecentAddressDialog()
{
    writeConfig();
}

void RecentAddressDialog::slotTypedSomething(const QString& text)
{
    if (mListView->currentItem()) {
        if (mListView->currentItem()->text() != mLineEdit->text() && !mLineEdit->text().isEmpty()) {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = mListView->signalsBlocked();
            mListView->blockSignals( true );
            QListWidgetItem *currentIndex = mListView->currentItem();
            if ( currentIndex ) {
                currentIndex->setText(text);
                mDirty = true;
            }
            mListView->blockSignals( block );
        }
    }
}

void RecentAddressDialog::slotAddItem()
{
    mListView->blockSignals(true);
    mListView->insertItem(0, QString());
    mListView->blockSignals(false);
    mListView->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
    mLineEdit->setFocus();
    mDirty = true;
    updateButtonState();
}

void RecentAddressDialog::slotRemoveItem()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    if (selectedItems.isEmpty())
        return;
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18np("Do you want to remove this email?", "Do you want to remove %1 emails?", selectedItems.count()), i18n("Remove"))) {
        Q_FOREACH(QListWidgetItem *item, selectedItems) {
            delete mListView->takeItem(mListView->row(item));
        }
        mDirty = true;
        updateButtonState();
    }
}

void RecentAddressDialog::updateButtonState()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    const int numberOfElementSelected(selectedItems.count());
    mRemoveButton->setEnabled(numberOfElementSelected);
    mNewButton->setEnabled(numberOfElementSelected <= 1);
    mLineEdit->setEnabled(numberOfElementSelected <= 1);

    if (numberOfElementSelected == 1) {
        const QString text = mListView->currentItem()->text();
        if (text != mLineEdit->text())
            mLineEdit->setText(text);
    } else {
        mLineEdit->clear();
    }
}

void RecentAddressDialog::slotSelectionChanged()
{
    updateButtonState();
}

void RecentAddressDialog::setAddresses( const QStringList &addrs )
{
    mListView->clear();
    mListView->addItems( addrs );
}

QStringList RecentAddressDialog::addresses() const
{
    QStringList lst;
    const int numberOfItem(mListView->count());
    for(int i = 0; i < numberOfItem; ++i) {
        lst<<mListView->item(i)->text();
    }
    return lst;
}

bool RecentAddressDialog::eventFilter( QObject* o, QEvent* e )
{
    if (o == mLineEdit && e->type() == QEvent::KeyPress ) {
        QKeyEvent* keyEvent = (QKeyEvent*)e;
        if (keyEvent->key() == Qt::Key_Down ||
                keyEvent->key() == Qt::Key_Up) {
            return ((QObject*)mListView)->event(e);
        }
    }

    return false;
}

void RecentAddressDialog::addAddresses(KConfig *config)
{
    const int numberOfItem(mListView->count());
    for (int i = 0; i < numberOfItem; ++i) {
        KPIM::RecentAddresses::self( config )->add( mListView->item(i)->text() );
    }
}

bool RecentAddressDialog::wasChanged() const
{
    return mDirty;
}

void RecentAddressDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void RecentAddressDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}
