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

#include "imagescalingwidget.h"
#include "ui_imagescalingwidget.h"
#include "messagecomposersettings.h"

#include <KComboBox>
#include <KLocale>
#include <KMessageBox>

#include <QImageWriter>

using namespace MessageComposer;

ImageScalingWidget::ImageScalingWidget(QWidget *parent)
  :QWidget(parent),
    ui(new Ui::ImageScalingWidget),
    mWasChanged(false)
{
  ui->setupUi(this);
  initComboBox(ui->CBMaximumWidth);
  initComboBox(ui->CBMaximumHeight);
  initComboBox(ui->CBMinimumWidth);
  initComboBox(ui->CBMinimumHeight);

  initWriteImageFormat();
  connect(ui->enabledAutoResize,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->KeepImageRatio,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->AskBeforeResizing,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->EnlargeImageToMinimum,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->ReduceImageToMaximum,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->customMaximumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMaximumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMinimumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMinimumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->skipImageSizeLower,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->imageSize,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->pattern,SIGNAL(textChanged(QString)),SIGNAL(changed()));
  connect(ui->CBMaximumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMaximumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMinimumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMinimumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->WriteToImageFormat,SIGNAL(activated(int)),SIGNAL(changed()));

  ui->pattern->setEnabled(false);
  mSourceFilterGroup = new QButtonGroup(ui->filterSourceGroupBox);
  connect( mSourceFilterGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotSourceFilterClicked(int)) );
  mSourceFilterGroup->addButton( ui->notFilterFilename, MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter );
  mSourceFilterGroup->addButton( ui->includeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern );
  mSourceFilterGroup->addButton( ui->excludeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern );
}

ImageScalingWidget::~ImageScalingWidget()
{
  delete ui;
}

void ImageScalingWidget::slotSourceFilterClicked(int button)
{
  ui->pattern->setEnabled(button != 0);
  Q_EMIT changed();
}

void ImageScalingWidget::slotComboboxChanged(int index)
{
  KComboBox* combo = qobject_cast< KComboBox* >( sender() );
  if (combo) {
    const bool isCustom = combo->itemData(index) == -1;
    if(combo == ui->CBMaximumWidth) {
      ui->customMaximumWidth->setEnabled(isCustom);
    } else if(combo == ui->CBMaximumHeight) {
      ui->customMaximumHeight->setEnabled(isCustom);
    } else if(combo == ui->CBMinimumWidth) {
      ui->customMinimumWidth->setEnabled(isCustom);
    } else if(combo == ui->CBMinimumHeight) {
      ui->customMinimumHeight->setEnabled(isCustom);
    }
    Q_EMIT changed();
  }
}

void ImageScalingWidget::initComboBox(KComboBox *combo)
{
  QList<int> size;
  size <<240
       <<320
       <<512
       <<640
       <<800
       <<1024
       <<1600
       <<2048;
  Q_FOREACH(int val, size) {
     combo->addItem(QString::number(val), val);
  }
  combo->addItem(i18n("Custom"), -1);
}

void ImageScalingWidget::initWriteImageFormat()
{
    /* Too many format :)
    QList<QByteArray> listWriteFormat = QImageWriter::supportedImageFormats();
    Q_FOREACH(const QByteArray& format, listWriteFormat) {
        ui->WriteToImageFormat->addItem(QString::fromLatin1(format));
    }
    */
    ui->WriteToImageFormat->addItem(QString::fromLatin1("JPG"));
    ui->WriteToImageFormat->addItem(QString::fromLatin1("PNG"));
}

void ImageScalingWidget::loadConfig()
{
  ui->enabledAutoResize->setChecked(MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled());
  ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
  ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
  ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
  ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());
  ui->skipImageSizeLower->setChecked(MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled());
  ui->imageSize->setValue(MessageComposer::MessageComposerSettings::self()->skipImageLowerSize());

  ui->customMaximumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumWidth());
  ui->customMaximumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumHeight());
  ui->customMinimumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumWidth());
  ui->customMinimumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumHeight());

  int index = qMax(0, ui->CBMaximumWidth->findData(MessageComposer::MessageComposerSettings::self()->maximumWidth()));
  ui->CBMaximumWidth->setCurrentIndex(index);
  ui->customMaximumWidth->setEnabled(ui->CBMaximumWidth->itemData(index) == -1);

  index = qMax(0, ui->CBMaximumHeight->findData(MessageComposer::MessageComposerSettings::self()->maximumHeight()));
  ui->CBMaximumHeight->setCurrentIndex(index);
  ui->customMaximumHeight->setEnabled(ui->CBMaximumHeight->itemData(index) == -1);

  index = qMax(0, ui->CBMinimumWidth->findData(MessageComposer::MessageComposerSettings::self()->minimumWidth()));
  ui->CBMinimumWidth->setCurrentIndex(index);
  ui->customMinimumWidth->setEnabled(ui->CBMinimumWidth->itemData(index) == -1);

  index = qMax(0, ui->CBMinimumHeight->findData(MessageComposer::MessageComposerSettings::self()->minimumHeight()));
  ui->CBMinimumHeight->setCurrentIndex(index);
  ui->customMinimumHeight->setEnabled(ui->CBMinimumHeight->itemData(index) == -1);

  index = ui->WriteToImageFormat->findData(MessageComposer::MessageComposerSettings::self()->writeFormat());
  if(index == -1) {
      ui->WriteToImageFormat->setCurrentIndex(0);
  } else {
      ui->WriteToImageFormat->setCurrentIndex(index);
  }
  ui->pattern->setText(MessageComposer::MessageComposerSettings::self()->filterSourcePattern());

  ui->renameResizedImage->setChecked(MessageComposer::MessageComposerSettings::self()->renameResizedImages());

  ui->renameResizedImagePattern->setText(MessageComposer::MessageComposerSettings::self()->renameResizedImagesPattern());

  updateFilterSourceTypeSettings();

  mWasChanged = false;
}

void ImageScalingWidget::updateFilterSourceTypeSettings()
{
    switch(MessageComposer::MessageComposerSettings::self()->filterSourceType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter:
        ui->notFilterFilename->setChecked(true);
        ui->pattern->setEnabled(false);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern:
        ui->includeFilesWithPattern->setChecked(true);
        ui->pattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern:
        ui->excludeFilesWithPattern->setChecked(true);
        ui->pattern->setEnabled(true);
        break;
    }
}

void ImageScalingWidget::writeConfig()
{
  if (ui->EnlargeImageToMinimum->isChecked() && ui->ReduceImageToMaximum->isChecked()) {
    if ((ui->customMinimumWidth->value()>=ui->customMaximumWidth->value()) ||
       (ui->customMinimumHeight->value()>=ui->customMaximumHeight->value())) {
        KMessageBox::error(this, i18n("Please verify minimum and maximum values."), i18n("Error in minimum Maximum value"));
        return;
    }
  }
  MessageComposer::MessageComposerSettings::self()->setAutoResizeImageEnabled(ui->enabledAutoResize->isChecked());
  MessageComposer::MessageComposerSettings::self()->setKeepImageRatio(ui->KeepImageRatio->isChecked());
  MessageComposer::MessageComposerSettings::self()->setAskBeforeResizing(ui->AskBeforeResizing->isChecked());
  MessageComposer::MessageComposerSettings::self()->setEnlargeImageToMinimum(ui->EnlargeImageToMinimum->isChecked());
  MessageComposer::MessageComposerSettings::self()->setReduceImageToMaximum(ui->ReduceImageToMaximum->isChecked());

  MessageComposer::MessageComposerSettings::self()->setCustomMaximumWidth(ui->customMaximumWidth->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMaximumHeight(ui->customMaximumHeight->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMinimumWidth(ui->customMinimumWidth->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMinimumHeight(ui->customMinimumHeight->value());

  MessageComposer::MessageComposerSettings::self()->setMaximumWidth(ui->CBMaximumWidth->itemData(ui->CBMaximumWidth->currentIndex()).toInt());
  MessageComposer::MessageComposerSettings::self()->setMaximumHeight(ui->CBMaximumHeight->itemData(ui->CBMaximumHeight->currentIndex()).toInt());
  MessageComposer::MessageComposerSettings::self()->setMinimumWidth(ui->CBMinimumWidth->itemData(ui->CBMinimumWidth->currentIndex()).toInt());
  MessageComposer::MessageComposerSettings::self()->setMinimumHeight(ui->CBMinimumHeight->itemData(ui->CBMinimumHeight->currentIndex()).toInt());

  MessageComposer::MessageComposerSettings::self()->setWriteFormat(ui->WriteToImageFormat->currentText());
  MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSizeEnabled(ui->skipImageSizeLower->isChecked());
  MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSize(ui->imageSize->value());

  MessageComposer::MessageComposerSettings::self()->setFilterSourcePattern(ui->pattern->text());

  MessageComposer::MessageComposerSettings::self()->setFilterSourceType(mSourceFilterGroup->checkedId());

  MessageComposer::MessageComposerSettings::self()->setRenameResizedImages(ui->renameResizedImage->isChecked());

  MessageComposer::MessageComposerSettings::self()->setRenameResizedImagesPattern(ui->renameResizedImagePattern->text());


  mWasChanged = false;
}

void ImageScalingWidget::resetToDefault()
{
   const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults( true );

   ui->enabledAutoResize->setChecked(MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled());
   ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
   ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
   ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
   ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());

   ui->customMaximumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumWidth());
   ui->customMaximumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumHeight());
   ui->customMinimumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumWidth());
   ui->customMinimumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumHeight());

   ui->skipImageSizeLower->setChecked(MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled());
   ui->imageSize->setValue(MessageComposer::MessageComposerSettings::self()->skipImageLowerSize());

   ui->pattern->setText(MessageComposer::MessageComposerSettings::self()->filterSourcePattern());

   int index = qMax(0, ui->CBMaximumWidth->findData(MessageComposer::MessageComposerSettings::self()->maximumWidth()));
   ui->CBMaximumWidth->setCurrentIndex(index);
   ui->customMaximumWidth->setEnabled(ui->CBMaximumWidth->itemData(index) == -1);

   index = qMax(0, ui->CBMaximumHeight->findData(MessageComposer::MessageComposerSettings::self()->maximumHeight()));
   ui->CBMaximumHeight->setCurrentIndex(index);
   ui->customMaximumHeight->setEnabled(ui->CBMaximumHeight->itemData(index) == -1);

   index = qMax(0, ui->CBMinimumWidth->findData(MessageComposer::MessageComposerSettings::self()->minimumWidth()));
   ui->CBMinimumWidth->setCurrentIndex(index);
   ui->customMinimumWidth->setEnabled(ui->CBMinimumWidth->itemData(index) == -1);

   index = qMax(0, ui->CBMinimumHeight->findData(MessageComposer::MessageComposerSettings::self()->minimumHeight()));
   ui->CBMinimumHeight->setCurrentIndex(index);
   ui->customMinimumHeight->setEnabled(ui->CBMinimumHeight->itemData(index) == -1);

   index = ui->WriteToImageFormat->findData(MessageComposer::MessageComposerSettings::self()->writeFormat());
   if (index == -1) {
      ui->WriteToImageFormat->setCurrentIndex(0);
   } else {
      ui->WriteToImageFormat->setCurrentIndex(index);
   }

   ui->renameResizedImage->setChecked(MessageComposer::MessageComposerSettings::self()->renameResizedImages());

   ui->renameResizedImagePattern->setText(MessageComposer::MessageComposerSettings::self()->renameResizedImagesPattern());

   updateFilterSourceTypeSettings();
   MessageComposer::MessageComposerSettings::self()->useDefaults( bUseDefaults );


   mWasChanged = false;
}

#include "imagescalingwidget.moc"
