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
#ifndef IMAGESCALINGJOB_H
#define IMAGESCALINGJOB_H

#include <QByteArray>
#include <QObject>
#include <QImage>
#include <QBuffer>

namespace MessageComposer {
class ImageScalingJob : public QObject
{
    Q_OBJECT
public:
    explicit ImageScalingJob(QObject *parent);
    ~ImageScalingJob();

    /**
     * @brief loadImageFromData
     * @param data
     * @return true if we can load image.
     */
    bool loadImageFromData(const QByteArray& data);

    /**
     * @brief resizeImage
     * @return true if we are able to resize image
     */
    bool resizeImage();

    /**
     * @brief imageArray
     * @return data from image after saving
     */
    QByteArray imageArray() const;

    /**
     * @brief mimetype
     * @return new image mimetype after saving.
     */
    QByteArray mimetype() const;

private:
    QImage mImage;
    QBuffer mBuffer;
};
}

#endif // IMAGESCALINGJOB_H
