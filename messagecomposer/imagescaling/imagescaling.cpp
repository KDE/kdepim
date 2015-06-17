/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#include "imagescaling.h"
#include "settings/messagecomposersettings.h"

using namespace MessageComposer;

ImageScaling::ImageScaling()
{
}

ImageScaling::~ImageScaling()
{
}

bool ImageScaling::loadImageFromData(const QByteArray &data)
{
    if (!mImage.loadFromData(data)) {
        return false;
    }
    return true;
}

bool ImageScaling::resizeImage()
{
    if (mImage.isNull()) {
        return false;
    }
    const int width = mImage.width();
    const int height = mImage.height();
    int newWidth = -1;
    int newHeight = -1;
    if (MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum()) {

        int maximumWidth = MessageComposer::MessageComposerSettings::self()->maximumWidth();
        if (maximumWidth == -1) {
            maximumWidth =  MessageComposer::MessageComposerSettings::self()->customMaximumWidth();
        }
        int maximumHeight = MessageComposer::MessageComposerSettings::self()->maximumHeight();
        if (maximumHeight == -1) {
            maximumHeight = MessageComposer::MessageComposerSettings::self()->customMaximumHeight();
        }
        if (width > maximumWidth) {
            newWidth = maximumWidth;
        } else {
            newWidth = width;
        }
        if (height > maximumHeight) {
            newHeight = maximumHeight;
        } else {
            newHeight = height;
        }
    } else {
        newHeight = height;
        newWidth = width;
    }
    if (MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum()) {

        int minimumWidth = MessageComposer::MessageComposerSettings::self()->minimumWidth();
        if (minimumWidth == -1) {
            minimumWidth =  MessageComposer::MessageComposerSettings::self()->customMinimumWidth();
        }

        int minimumHeight = MessageComposer::MessageComposerSettings::self()->minimumHeight();
        if (minimumHeight == -1) {
            minimumHeight = MessageComposer::MessageComposerSettings::self()->customMinimumHeight();
        }
        if (newWidth < minimumWidth) {
            newWidth = minimumWidth;
        }
        if (newHeight < minimumHeight) {
            newHeight = minimumHeight;
        }
    }
    if ((newHeight != height) || (newWidth != width)) {
        mBuffer.open(QIODevice::WriteOnly);
        mImage = mImage.scaled(newWidth, newHeight, MessageComposer::MessageComposerSettings::self()->keepImageRatio() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

        QByteArray format;
        if (mMimeType == "image/jpeg") {
            format = "JPG";
        } else if (mMimeType == "image/png") {
            format = "PNG";
        } else {
            format = MessageComposer::MessageComposerSettings::self()->writeFormat().toLocal8Bit();
        }
        const bool result = mImage.save(&mBuffer, format);
        mBuffer.close();
        return result;
    } else {
        return false;
    }
    return true;

}

QByteArray ImageScaling::mimetype() const
{
    if (mMimeType.isEmpty()) {
        return QByteArray();
    }
    if ((mMimeType == "image/jpeg") || (mMimeType == "image/png")) {
        return mMimeType;
    } else {
        //Add more mimetype if a day we add more saving format.
        const QString type = MessageComposer::MessageComposerSettings::self()->writeFormat();
        if (type == QLatin1String("JPG")) {
            return "image/jpeg";
        } else if (type == QLatin1String("PNG")) {
            return "image/png";
        }
    }
    return QByteArray();
}

void ImageScaling::setMimetype(const QByteArray &mimetype)
{
    mMimeType = mimetype;
}

void ImageScaling::setName(const QString &name)
{
    mName = name;
}

QByteArray ImageScaling::imageArray() const
{
    return mBuffer.data();
}

QString ImageScaling::generateNewName()
{
    if (mName.isEmpty()) {
        return QString();
    }

    // Don't rename it.
    if ((mMimeType == "image/jpeg") || (mMimeType == "image/png")) {
        return mName;
    }
    const QString type = MessageComposer::MessageComposerSettings::self()->writeFormat();
    if (mName.endsWith(QLatin1String(".png"))) {
        if (type != QLatin1String("PNG")) {
            mName.replace(QStringLiteral(".png"), QStringLiteral(".jpg"));
        }
    } else if (mName.endsWith(QLatin1String(".jpg"))) {
        if (type != QLatin1String("JPG")) {
            mName.replace(QStringLiteral(".jpg"), QStringLiteral(".png"));
        }
    } else {
        if (type == QLatin1String("PNG")) {
            mName += QLatin1String(".png");
        } else {
            mName += QLatin1String(".jpg");
        }
    }
    return mName;
}
