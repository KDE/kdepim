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

  connect(ui->enabledAutoResize,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->KeepImageRatio,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->AskBeforeResizing,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->EnlargeImageToMinimum,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->ReduceImageToMaximum,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->customMaximumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMaximumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMinimumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
  connect(ui->customMinimumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));

  connect(ui->CBMaximumWidth,SIGNAL(activated(QString)),SLOT(slotComboboxChanged(QString)));
  connect(ui->CBMaximumHeight,SIGNAL(activated(QString)),SLOT(slotComboboxChanged(QString)));
  connect(ui->CBMinimumWidth,SIGNAL(activated(QString)),SLOT(slotComboboxChanged(QString)));
  connect(ui->CBMinimumHeight,SIGNAL(activated(QString)),SLOT(slotComboboxChanged(QString)));
}

AutoResizeImageWidget::~AutoResizeImageWidget()
{
  delete ui;
}

void AutoResizeImageWidget::slotComboboxChanged(const QString& text)
{
  const bool isCustom = (text == i18n("Custom"));
  KComboBox* combo = qobject_cast< KComboBox* >( sender() );
  if(combo) {
    if(combo == ui->CBMaximumWidth) {
      ui->customMaximumWidth->setEnabled(isCustom);
    } else if(combo == ui->CBMaximumHeight) {
      ui->customMaximumHeight->setEnabled(isCustom);
    } else if(combo == ui->CBMinimumWidth) {
      ui->customMinimumWidth->setEnabled(isCustom);
    } else if(combo == ui->CBMinimumHeight) {
      ui->customMinimumHeight->setEnabled(isCustom);
    }
  }
  Q_EMIT changed();
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
     combo->addItem(QString::number(val));
  }
  combo->addItem(i18n("Custom"));
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
  mWasChanged = false;
}

void AutoResizeImageWidget::writeConfig()
{
  MessageComposer::MessageComposerSettings::self()->setAutoResizeImageEnabled(ui->enabledAutoResize->isChecked());
  MessageComposer::MessageComposerSettings::self()->setKeepImageRatio(ui->KeepImageRatio->isChecked());
  MessageComposer::MessageComposerSettings::self()->setAskBeforeResizing(ui->AskBeforeResizing->isChecked());
  MessageComposer::MessageComposerSettings::self()->setEnlargeImageToMinimum(ui->EnlargeImageToMinimum->isChecked());
  MessageComposer::MessageComposerSettings::self()->setReduceImageToMaximum(ui->ReduceImageToMaximum->isChecked());

  MessageComposer::MessageComposerSettings::self()->setCustomMaximumWidth(ui->customMaximumWidth->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMaximumHeight(ui->customMaximumHeight->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMinimumWidth(ui->customMinimumWidth->value());
  MessageComposer::MessageComposerSettings::self()->setCustomMinimumHeight(ui->customMinimumHeight->value());

  mWasChanged = false;
}

void AutoResizeImageWidget::resetToDefault()
{
  mWasChanged = false;
}

#include "autoresizeimagewidget.moc"
