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
#include "autoresizeimagejob.h"
#include "messagecomposersettings.h"

#include <QImage>

AutoResizeImageJob::AutoResizeImageJob(QObject *parent)
    :QObject(parent)
{
}

AutoResizeImageJob::~AutoResizeImageJob()
{

}

bool AutoResizeImageJob::loadImageFromData(const QByteArray& data)
{
  if(!mImage.loadFromData(data)) {
    return false;
  }
  return true;
}

bool AutoResizeImageJob::resizeImage()
{
  if(mImage.isNull())
     return false;
  const int width = mImage.width();
  const int height = mImage.height();
  /*
  if(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum()) {
      if(width < )
  }
  ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
  ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
  ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
  ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());

  ui->customMaximumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumWidth());
  ui->customMaximumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumHeight());
  ui->customMinimumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumWidth());
  ui->customMinimumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumHeight());


  ui->CBMaximumWidth->setCurrentIndex(MessageComposer::MessageComposerSettings::self()->maximumWidth());
  ui->CBMaximumHeight->setCurrentIndex(MessageComposer::MessageComposerSettings::self()->maximumHeight());
  ui->CBMinimumWidth->setCurrentIndex(MessageComposer::MessageComposerSettings::self()->minimumWidth());
  ui->CBMinimumHeight->setCurrentIndex(MessageComposer::MessageComposerSettings::self()->minimumHeight());
  */
  return true;

}

#include "autoresizeimagejob.moc"
