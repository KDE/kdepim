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

class MessageComposer::ImageScalingPrivate
{
public:
    ImageScalingPrivate()
    {

    }

    QImage mImage;
    QBuffer mBuffer;
    QString mName;
    QByteArray mMimeType;
};

ImageScaling::ImageScaling()
    : d(new MessageComposer::ImageScalingPrivate)
{
}

ImageScaling::~ImageScaling()
{
    delete d;
}

bool ImageScaling::loadImageFromData(const QByteArray &data)
{
    if (!d->mImage.loadFromData(data)) {
        return false;
    }
    return true;
}

bool ImageScaling::resizeImage()
{
    if (d->mImage.isNull()) {
        return false;
    }
    const int width = d->mImage.width();
    const int height = d->mImage.height();
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
        d->mBuffer.open(QIODevice::WriteOnly);
        d->mImage = d->mImage.scaled(newWidth, newHeight, MessageComposer::MessageComposerSettings::self()->keepImageRatio() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

        QByteArray format;
        if (d->mMimeType == "image/jpeg") {
            format = "JPG";
        } else if (d->mMimeType == "image/png") {
            format = "PNG";
        } else {
            format = MessageComposer::MessageComposerSettings::self()->writeFormat().toLocal8Bit();
            if (format.isEmpty()) {
                format = "PNG";
            }
        }
        const bool result = d->mImage.save(&d->mBuffer, format);
        d->mBuffer.close();
        return result;
    } else {
        return false;
    }
    return true;

}

QByteArray ImageScaling::mimetype() const
{
    if (d->mMimeType.isEmpty()) {
        return QByteArray();
    }
    if ((d->mMimeType == "image/jpeg") || (d->mMimeType == "image/png")) {
        return d->mMimeType;
    } else {
        //Add more mimetype if a day we add more saving format.
        const QString type = MessageComposer::MessageComposerSettings::self()->writeFormat();
        if (type == QLatin1String("JPG")) {
            return "image/jpeg";
        } else if (type == QLatin1String("PNG")) {
            return "image/png";
        } else {
            return "image/png";
        }
    }
    return QByteArray();
}

void ImageScaling::setMimetype(const QByteArray &mimetype)
{
    d->mMimeType = mimetype;
}

void ImageScaling::setName(const QString &name)
{
    d->mName = name;
}

QByteArray ImageScaling::imageArray() const
{
    return d->mBuffer.data();
}

QString ImageScaling::generateNewName()
{
    if (d->mName.isEmpty()) {
        return QString();
    }

    // Don't rename it.
    if ((d->mMimeType == "image/jpeg") || (d->mMimeType == "image/png")) {
        return d->mName;
    }
    QString type = MessageComposer::MessageComposerSettings::self()->writeFormat();
    if (type.isEmpty()) {
        type = QStringLiteral("PNG");
    }
    if (d->mName.endsWith(QStringLiteral(".png"))) {
        if (type != QLatin1String("PNG")) {
            d->mName.replace(QStringLiteral(".png"), QStringLiteral(".jpg"));
        }
    } else if (d->mName.endsWith(QStringLiteral(".jpg"))) {
        if (type != QLatin1String("JPG")) {
            d->mName.replace(QStringLiteral(".jpg"), QStringLiteral(".png"));
        }
    } else {
        if (type == QLatin1String("PNG")) {
            d->mName += QLatin1String(".png");
        } else {
            d->mName += QLatin1String(".jpg");
        }
    }
    return d->mName;
}
