/*
 * Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Tobias Koenig <tokoe@kdab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "aclentrydialog_p.h"
#include "aclutils_p.h"

#include "addressline/addresseelineedit.h"

#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KLocalizedString>

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace PimCommon;

class AclEntryDialog::Private
{
public:
    Private( AclEntryDialog *qq )
        : q( qq ),
          mButtonGroup(0),
          mUserIdLineEdit(0),
          mButtonLayout(0)
    {
    }

    void slotChanged();
    void slotSelectAddresses();

    AclEntryDialog *q;
    QButtonGroup *mButtonGroup;
    KPIM::AddresseeLineEdit *mUserIdLineEdit;
    QVBoxLayout *mButtonLayout;
    KIMAP::Acl::Rights mCustomPermissions;
    QPushButton *mOkButton;
};

void AclEntryDialog::Private::slotChanged()
{
    mOkButton->setEnabled( !mUserIdLineEdit->text().isEmpty() && mButtonGroup->checkedButton() != 0 );
}

void AclEntryDialog::Private::slotSelectAddresses()
{
    Akonadi::EmailAddressSelectionDialog dlg;

    if ( !dlg.exec() ) {
        return;
    }

    const QString text = !dlg.selectedAddresses().isEmpty() ?
                dlg.selectedAddresses().first().quotedEmail() :
                QString();

    mUserIdLineEdit->setText( text );
}

AclEntryDialog::AclEntryDialog( QWidget *parent )
    : QDialog( parent ), d( new Private( this ) )
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AclEntryDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AclEntryDialog::reject);

    QWidget *page = new QWidget( this );
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    QGridLayout *layout = new QGridLayout( page );
    //QT5 port layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    QLabel *label = new QLabel( i18n( "&User identifier:" ), page );
    layout->addWidget( label, 0, 0 );

    d->mUserIdLineEdit = new KPIM::AddresseeLineEdit( page );
    layout->addWidget( d->mUserIdLineEdit, 0, 1 );
    label->setBuddy( d->mUserIdLineEdit );
    d->mUserIdLineEdit->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "The User Identifier is the login of the user on the IMAP server. "
                       "This can be a simple user name or the full email address of the user; "
                       "the login for your own account on the server will tell you which one it is." ) );

    QPushButton *button = new QPushButton( i18nc( "select an email address", "Se&lect..." ), page );
    layout->addWidget( button, 0, 2 );

    QGroupBox *groupBox = new QGroupBox( i18n( "Permissions" ), page );
    d->mButtonLayout = new QVBoxLayout( groupBox );

    d->mButtonGroup = new QButtonGroup( groupBox );

    for ( unsigned int i = 0; i < AclUtils::standardPermissionsCount(); ++i ) {
        const KIMAP::Acl::Rights permissions = AclUtils::permissionsForIndex( i );

        QRadioButton *radioButton =
                new QRadioButton( AclUtils::permissionsToUserString( permissions ), groupBox );
        d->mButtonLayout->addWidget( radioButton );
        d->mButtonGroup->addButton( radioButton, permissions );
    }

    d->mButtonLayout->addStretch( 1 );
    layout->addWidget( groupBox, 1, 0, 1, 3 );

    label =
            new QLabel(
                i18n( "<b>Note: </b>Renaming requires write permissions on the parent folder." ), page );
    layout->addWidget( label, 2, 0, 1, 3 );
    layout->setRowStretch( 2, 10 );

    connect( d->mUserIdLineEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()) );
    connect( button, SIGNAL(clicked()), SLOT(slotSelectAddresses()) );
    connect( d->mButtonGroup, SIGNAL(buttonClicked(int)), SLOT(slotChanged()) );

    d->mOkButton->setEnabled( false );

    d->mUserIdLineEdit->setFocus();

    // Ensure the lineedit is rather wide so that email addresses can be read in it
    //QT5 port incrementInitialSize( QSize( 200, 0 ) );
}

AclEntryDialog::~AclEntryDialog()
{
    delete d;
}

void AclEntryDialog::setUserId( const QString &userId )
{
    d->mUserIdLineEdit->setText( userId );

    d->mOkButton->setEnabled( !userId.isEmpty() );
}

QString AclEntryDialog::userId() const
{
    return d->mUserIdLineEdit->text();
}

void AclEntryDialog::setPermissions( KIMAP::Acl::Rights permissions )
{
    QAbstractButton *button = d->mButtonGroup->button( KIMAP::Acl::normalizedRights( permissions ) );

    if ( button ) {
        button->setChecked( true );
    } else {
        QRadioButton *radioButton =
                new QRadioButton( AclUtils::permissionsToUserString( permissions ) );

        d->mButtonLayout->addWidget( radioButton );
        d->mButtonGroup->addButton( radioButton, permissions );
    }

    d->mCustomPermissions = permissions;
}

KIMAP::Acl::Rights AclEntryDialog::permissions() const
{
    QAbstractButton *button = d->mButtonGroup->checkedButton();

    if ( !button ) {
        return d->mCustomPermissions;
    }

    return KIMAP::Acl::denormalizedRights(
                static_cast<KIMAP::Acl::Rights>( d->mButtonGroup->id( button ) ) );
}

#include "moc_aclentrydialog_p.cpp"
