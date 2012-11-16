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
  if(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum()) {
      int maximumWidth = MessageComposer::MessageComposerSettings::self()->maximumWidth();
      if (maximumWidth == -1) {
          maximumWidth =  MessageComposer::MessageComposerSettings::self()->customMaximumWidth();
      }
      int maximumHeight = MessageComposer::MessageComposerSettings::self()->maximumHeight();
      if (maximumHeight == -1) {
          maximumHeight = MessageComposer::MessageComposerSettings::self()->customMaximumHeight();
      }
      if((width < maximumWidth) && (height < maximumHeight)) {
          return true;
      }
  }

  if(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum()) {
      int minimumWidth = MessageComposer::MessageComposerSettings::self()->minimumWidth();
      if (minimumWidth == -1) {
          minimumWidth =  MessageComposer::MessageComposerSettings::self()->customMinimumWidth();
      }

      int minimumHeight = MessageComposer::MessageComposerSettings::self()->minimumHeight();
      if (minimumHeight == -1) {
          minimumHeight = MessageComposer::MessageComposerSettings::self()->customMinimumHeight();
      }
      if((width > minimumWidth) && (height > minimumHeight)) {
          return true;
      }

  }
  /*
  ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
  ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
  */
  return true;

}

#include "autoresizeimagejob.moc"
