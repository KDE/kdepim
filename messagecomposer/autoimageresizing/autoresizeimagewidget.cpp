/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "autoresizeimagewidget.h"
#include "ui_autoresizeimagewidget.h"
#include "messagecomposersettings.h"

#include <KComboBox>
#include <KLocale>
#include <KMessageBox>

#include <QImageWriter>

using namespace MessageComposer;

AutoResizeImageWidget::AutoResizeImageWidget(QWidget *parent)
  :QWidget(parent),
    ui(new Ui::AutoResizeImageWidget),
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

  connect(ui->CBMaximumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMaximumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMinimumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->CBMinimumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
  connect(ui->WriteToImageFormat,SIGNAL(activated(int)),SIGNAL(changed()));
}

AutoResizeImageWidget::~AutoResizeImageWidget()
{
  delete ui;
}

void AutoResizeImageWidget::slotComboboxChanged(int index)
{
  KComboBox* combo = qobject_cast< KComboBox* >( sender() );
  if(combo) {
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

void AutoResizeImageWidget::initComboBox(KComboBox *combo)
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
  Q_FOREACH(int val, size)
  {
     combo->addItem(QString::number(val), val);
  }
  combo->addItem(i18n("Custom"), -1);
}

void AutoResizeImageWidget::initWriteImageFormat()
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

void AutoResizeImageWidget::loadConfig()
{
  ui->enabledAutoResize->setChecked(MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled());
  ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
  ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
  ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
  ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());

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
  mWasChanged = false;
}

void AutoResizeImageWidget::writeConfig()
{
  if(ui->EnlargeImageToMinimum->isChecked() && ui->ReduceImageToMaximum->isChecked()) {
    if((ui->customMinimumWidth->value()>=ui->customMaximumWidth->value()) ||
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

  mWasChanged = false;
}

void AutoResizeImageWidget::resetToDefault()
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


   MessageComposer::MessageComposerSettings::self()->useDefaults( bUseDefaults );
}

#include "autoresizeimagewidget.moc"
