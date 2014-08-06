/*
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactselectiondialog.h"
#include "contactselectionwidget.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

ContactSelectionDialog::ContactSelectionDialog( QItemSelectionModel *selectionModel,
                                                QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( i18n( "Select Contacts" ) );
    //PORTING SCRIPT: Move QDialogButtonBox at the end of init of widget to add it in layout.
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ContactSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ContactSelectionDialog::reject);

    mSelectionWidget = new ContactSelectionWidget( selectionModel, this );
    mainLayout->addWidget(mSelectionWidget);
    mainLayout->addWidget(buttonBox);


    resize( QSize( 450, 220 ) );
}

void ContactSelectionDialog::setMessageText( const QString &message )
{
    mSelectionWidget->setMessageText( message );
}

void ContactSelectionDialog::setDefaultAddressBook( const Akonadi::Collection &addressBook )
{
    mSelectionWidget->setDefaultAddressBook( addressBook );
}

KABC::Addressee::List ContactSelectionDialog::selectedContacts() const
{
    return mSelectionWidget->selectedContacts();
}

