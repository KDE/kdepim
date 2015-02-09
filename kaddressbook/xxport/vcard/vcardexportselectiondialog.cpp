/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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


#include "vcardexportselectiondialog.h"

#include <QFrame>
#include <KLocalizedString>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <kconfig.h>
#include <KConfigGroup>

// ---------- VCardExportSelection Dialog ---------------- //

VCardExportSelectionDialog::VCardExportSelectionDialog( QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18nc( "@title:window", "Select vCard Fields" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    QFrame *page = new QFrame( this );
    setMainWidget( page );

    QGridLayout *layout = new QGridLayout( page );
    layout->setSpacing( spacingHint() );
    layout->setMargin( marginHint() );

    QGroupBox *gbox = new QGroupBox(
                i18nc( "@title:group", "Fields to be exported" ), page );
    gbox->setFlat( true );
    layout->addWidget( gbox, 0, 0, 1, 2 );

    mPrivateBox = new QCheckBox( i18nc( "@option:check", "Private fields" ), page );
    mPrivateBox->setToolTip(
                i18nc( "@info:tooltip", "Export private fields" ) );
    mPrivateBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's "
                       "private fields to the vCard output file." ) );
    layout->addWidget( mPrivateBox, 1, 0 );

    mBusinessBox = new QCheckBox( i18nc( "@option:check", "Business fields" ), page );
    mBusinessBox->setToolTip(
                i18nc( "@info:tooltip", "Export business fields" ) );
    mBusinessBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's "
                       "business fields to the vCard output file." ) );
    layout->addWidget( mBusinessBox, 2, 0 );

    mOtherBox = new QCheckBox( i18nc( "@option:check", "Other fields" ), page );
    mOtherBox->setToolTip(
                i18nc( "@info:tooltip", "Export other fields" ) );
    mOtherBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's "
                       "other fields to the vCard output file." ) );
    layout->addWidget( mOtherBox, 3, 0 );

    mEncryptionKeys = new QCheckBox( i18nc( "@option:check", "Encryption keys" ), page );
    mEncryptionKeys->setToolTip(
                i18nc( "@info:tooltip", "Export encryption keys" ) );
    mEncryptionKeys->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's "
                       "encryption keys to the vCard output file." ) );
    layout->addWidget( mEncryptionKeys, 1, 1 );

    mPictureBox = new QCheckBox( i18nc( "@option:check", "Pictures" ), page );
    mPictureBox->setToolTip(
                i18nc( "@info:tooltip", "Export pictures" ) );
    mPictureBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's "
                       "picture to the vCard output file." ) );
    layout->addWidget( mPictureBox, 2, 1 );

    gbox = new QGroupBox(
                i18nc( "@title:group", "Export options" ), page );
    gbox->setFlat( true );
    layout->addWidget( gbox, 4, 0, 1, 2 );

    mDisplayNameBox = new QCheckBox( i18nc( "@option:check", "Display name as full name" ), page );
    mDisplayNameBox->setToolTip(
                i18nc( "@info:tooltip", "Export display name as full name" ) );
    mDisplayNameBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to export the contact's display name "
                       "in the vCard's full name field.  This may be required to get the "
                       "name shown correctly in GMail or Android." ) );
    layout->addWidget( mDisplayNameBox, 5, 0, 1, 2 );

    KConfig config( QLatin1String("kaddressbookrc") );
    const KConfigGroup group( &config, "XXPortVCard" );

    mPrivateBox->setChecked( group.readEntry( "ExportPrivateFields", true ) );
    mBusinessBox->setChecked( group.readEntry( "ExportBusinessFields", true ) );
    mOtherBox->setChecked( group.readEntry( "ExportOtherFields", true ) );
    mEncryptionKeys->setChecked( group.readEntry( "ExportEncryptionKeys", true ) );
    mPictureBox->setChecked( group.readEntry( "ExportPictureFields", true ) );
    mDisplayNameBox->setChecked( group.readEntry( "ExportDisplayName", false ) );
}

VCardExportSelectionDialog::~VCardExportSelectionDialog()
{
    KConfig config( QLatin1String("kaddressbookrc") );
    KConfigGroup group( &config, "XXPortVCard" );

    group.writeEntry( "ExportPrivateFields", mPrivateBox->isChecked() );
    group.writeEntry( "ExportBusinessFields", mBusinessBox->isChecked() );
    group.writeEntry( "ExportOtherFields", mOtherBox->isChecked() );
    group.writeEntry( "ExportEncryptionKeys", mEncryptionKeys->isChecked() );
    group.writeEntry( "ExportPictureFields", mPictureBox->isChecked() );
    group.writeEntry( "ExportDisplayName", mDisplayNameBox->isChecked() );
}

VCardExportSelectionDialog::ExportFields VCardExportSelectionDialog::exportType() const
{
    VCardExportSelectionDialog::ExportFields type = None;
    if (mPrivateBox->isChecked()) {
        type |= Private;
    }
    if (mBusinessBox->isChecked()) {
        type |= Business;
    }
    if (mOtherBox->isChecked()) {
        type |= Other;
    }
    if (mEncryptionKeys->isChecked()) {
        type |= Encryption;
    }
    if (mPictureBox->isChecked()) {
        type |= Picture;
    }
    if (mDisplayNameBox->isChecked()) {
        type |= DiplayName;
    }
    return type;
}

