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

#include "imagescalingdialog.h"
#include <KLocale>

#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSpinBox>

using namespace MessageComposer;

ImageScalingDialog::ImageScalingDialog(QWidget *parent)
    :KDialog(parent), mImageRatio(-1)
{
    setCaption( i18nc("@title:window", "Resize Image") );
    setButtons( User1 | Cancel );
    setDefaultButton( User1 );
    setModal( false );
    setButtonText( User1, i18nc("@action:button","Resize") );


    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QLabel *lab = new QLabel(this);
    QVBoxLayout *lay = new QVBoxLayout( page );
    lay->addWidget( lab );

    mKeepOriginalSize = new QCheckBox( i18n( "Keep Original Size" ) );
    connect( mKeepOriginalSize, SIGNAL(clicked(bool)), this, SLOT(slotKeepOriginalSizeClicked(bool)) );
    mKeepOriginalSize->setChecked( true );
    lay->addWidget( mKeepOriginalSize );

    mKeepImageRatio = new QCheckBox( i18n( "Keep Image Ratio" ) );
    mKeepImageRatio->setChecked( true );
    mKeepImageRatio->setEnabled( false );
    lay->addWidget( mKeepImageRatio );

    QHBoxLayout *hbox = new QHBoxLayout;
    lab = new QLabel( i18n( "Width:" ) );
    mWidth = new QSpinBox;
    mWidth->setMinimum( 1 );
    mWidth->setMaximum( 99999 );
    mWidth->setEnabled( false );
    mWidth->setSuffix( i18n( " px" ) );
    lab->setBuddy( mWidth );
    connect( mWidth, SIGNAL(valueChanged(int)), this, SLOT(slotImageWidthChanged(int)) );

    hbox->addWidget( lab );
    hbox->addWidget( mWidth );
    lay->addLayout( hbox );

    hbox = new QHBoxLayout;
    lab = new QLabel( i18n( "Height:" ) );
    mHeight = new QSpinBox;
    mHeight->setMinimum( 1 );
    mHeight->setMaximum( 99999 );
    mHeight->setEnabled( false );
    mHeight->setSuffix( i18n( " px" ) );
    lab->setBuddy( mHeight );
    connect( mHeight, SIGNAL(valueChanged(int)), this, SLOT(slotImageHeightChanged(int)) );
    hbox->addWidget( lab );
    hbox->addWidget( mHeight );
    lay->addLayout( hbox );


    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
}

ImageScalingDialog::~ImageScalingDialog()
{
}

void ImageScalingDialog::setImageFromData(const QByteArray& data)
{
    //TODO add KMessageBox when error.
    if(!mImage.loadFromData(data)) {
      //return false;
    }
    //return true;
}

QByteArray ImageScalingDialog::imageData()
{
    return mBuffer.data();
}

void ImageScalingDialog::slotUser1()
{
    mBuffer.open(QIODevice::WriteOnly);
    mImage = mImage.scaled(mWidth->value(),mHeight->value());
    const bool result = mImage.save(&mBuffer,"PNG"); //TODO customize it.

    Q_UNUSED( result );

    mBuffer.close();
    accept();
}

void ImageScalingDialog::slotKeepOriginalSizeClicked(bool checked)
{
    mHeight->setEnabled( !checked );
    mWidth->setEnabled( !checked );
    mKeepImageRatio->setEnabled( !checked );
}

void ImageScalingDialog::slotImageWidthChanged(int value)
{
    if ( mKeepImageRatio->isChecked() && !mKeepOriginalSize->isChecked() ) {
        if ( mImageRatio != -1 ) {
            mHeight->blockSignals( true );
            mHeight->setValue( value * mImageRatio );
            mHeight->blockSignals( false );
        }
    }
}

void ImageScalingDialog::slotImageHeightChanged(int value)
{
    if ( mKeepImageRatio->isChecked()&& !mKeepOriginalSize->isChecked() ) {
        if ( mImageRatio != -1 ) {
            mWidth->blockSignals( true );
            mWidth->setValue( value / mImageRatio );
            mWidth->blockSignals( false );
        }
    }
}

QByteArray ImageScalingDialog::mimetype() const
{
    return "image/png"; //Customize it too
}

#include "imagescalingdialog.moc"
