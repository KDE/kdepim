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

#include <QBuffer>
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
  //const qreal imageRatio = (double)( (double)height / (double)width );
  int newWidth = -1;
  int newHeight = -1;
  if(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum()) {

      int maximumWidth = MessageComposer::MessageComposerSettings::self()->maximumWidth();
      if (maximumWidth == -1) {
          maximumWidth =  MessageComposer::MessageComposerSettings::self()->customMaximumWidth();
      }
      int maximumHeight = MessageComposer::MessageComposerSettings::self()->maximumHeight();
      if (maximumHeight == -1) {
          maximumHeight = MessageComposer::MessageComposerSettings::self()->customMaximumHeight();
      }

      /*
      if(MessageComposer::MessageComposerSettings::self()->keepImageRatio()) {
          if(imageRatio>1) {

          } else {

          }
         //TODO
      } else {
      */
          if( width > maximumWidth ) {
              newWidth = maximumWidth;
          } else {
              newWidth = width;
          }
          if(height > maximumHeight) {
              newHeight = maximumHeight;
          } else {
              newHeight = height;
          }
      //}
  } else {
      newHeight = height;
      newWidth = width;
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
/*
      if(MessageComposer::MessageComposerSettings::self()->keepImageRatio()) {
          if(imageRatio>1) {

          } else {

          }
          //TODO
      } else {
      */
          if(newWidth < minimumWidth) {
              newWidth = minimumWidth;
          }
          if(newHeight < minimumHeight) {
              newHeight = minimumHeight;
          }
      //}
  }
  if((newHeight != height) || (newWidth != width)) {
      mBuffer.open(QIODevice::WriteOnly);
      mImage = mImage.scaled(newWidth,newHeight, MessageComposer::MessageComposerSettings::self()->keepImageRatio() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
      const bool result = mImage.save(&mBuffer,MessageComposer::MessageComposerSettings::self()->writeFormat().toLocal8Bit());
      mBuffer.close();
      return result;
  } else {
      return false;
  }
  return true;

}

QByteArray AutoResizeImageJob::imageArray() const
{
    return mBuffer.data();
}

#include "autoresizeimagejob.moc"
