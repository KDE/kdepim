/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef RICHTEXTCOMPOSERIMAGES_H
#define RICHTEXTCOMPOSERIMAGES_H

#include <QImage>
#include <QObject>
#include <QFileInfo>
#include <QTextImageFormat>
#include <QSharedPointer>
#include "messagecomposer_export.h"
namespace MessageComposer
{
class RichTextComposer;
/**
 * Holds information about an embedded HTML image that will be useful for mail clients.
 * A list with all images can be retrieved with TextEdit::embeddedImages().
 */
struct EmbeddedImage {
    QByteArray image;   ///< The image, encoded as PNG with base64 encoding
    QString contentID;  ///< The content id of the embedded image
    QString imageName;  ///< Name of the image as it is available as a resource in the editor
};

/**
 * Holds information about an embedded HTML image that will be generally useful.
 * A list with all images can be retrieved with TextEdit::imagesWithName().
 *
 * @since 4.4
 */
struct ImageWithName {
    QImage image; ///< The image
    QString name; ///< The name of the image as it is available as a resource in the editor
};

typedef QSharedPointer<ImageWithName> ImageWithNamePtr;
typedef QList< ImageWithNamePtr > ImageWithNameList;
typedef QList< QSharedPointer<EmbeddedImage> > ImageList;

class MESSAGECOMPOSER_EXPORT RichTextComposerImages : public QObject
{
    Q_OBJECT
public:
    explicit RichTextComposerImages(RichTextComposer *composer, QObject *parent = Q_NULLPTR);
    ~RichTextComposerImages();

    /**
     * Adds an image. The image is loaded from file and then pasted to the current
     * cursor position with the given @p width and @p height.
     *
     * @param url The URL of the file which contains the image
     * @param width The width the inserted image will have.
     * @param height The height the inserted image will have.
     *
     */
    void addImage(const QUrl &url, int width = -1, int height = -1);

    /**
     * Loads an image into the textedit. The difference to addImage() is that this
     * function expects that the image tag is already present in the HTML source.
     *
     * @param image the image to load
     * @param matchName the name of tags to match image
     * @param resourceName the resource name of image
     * So what this message does is that it scans the HTML source for the image
     * tag that matches the @p matchName, and then inserts the @p image as a
     * resource, giving that resource the name @p resourceName.
     *
     */
    void loadImage(const QImage &image, const QString &matchName, const QString &resourceName);

    void addImageHelper(const QString &imageName, const QImage &image, int width = -1, int height = -1);
    ImageWithNameList imagesWithName() const;
    QList<QSharedPointer<EmbeddedImage> > embeddedImages() const;
    QList<QTextImageFormat> embeddedImageFormats() const;
    void addImageHelper(const QUrl &url, int width = -1, int height = -1);
    void insertImage(const QImage &image, const QFileInfo &fileInfo);

    /**
     * For all given embedded images, this function replace the image name
     * in the <img> tag of the HTML body with cid:content-id, so that the
     * HTML references the image body parts, see RFC 2557.
     *
     * This is useful when building a MIME message with inline images.
     *
     * Note that this function works on encoded content already.
     *
     * @param htmlBody the HTML code in which the <img> tag will be modified.
     *                 The HTML code here could come from toHtml(), for example.
     *
     * @param imageList the list of images of which the <img> tag will be modified.
     *                  You can get such a list from the embeddedImages() function.
     *
     * @return a modified HTML code, where the <img> tags got replaced
     */
    static QByteArray imageNamesToContentIds(const QByteArray &htmlBody, const ImageList &imageList);

private:
    class RichTextComposerImagesPrivate;
    RichTextComposerImagesPrivate *const d;
};
}

#endif // RICHTEXTCOMPOSERIMAGES_H
