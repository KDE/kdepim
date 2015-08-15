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

#include "richtextcomposer.h"
#include "richtextcomposerimages.h"

#include <QTextBlock>
#include <QTextDocument>
#include <QBuffer>
#include <QDateTime>
#include <KCodecs>
#include <KMessageBox>
#include <KLocalizedString>
#include <QFileInfo>

using namespace MessageComposer;

class RichTextComposerImages::RichTextComposerImagesPrivate
{
public:
    RichTextComposerImagesPrivate(RichTextComposer *editor)
        : composer(editor)
    {

    }
    /**
     * The names of embedded images.
     * Used to easily obtain the names of the images.
     * New images are compared to the list and not added as resource if already present.
     */
    QStringList mImageNames;

    RichTextComposer *composer;
};

RichTextComposerImages::RichTextComposerImages(RichTextComposer *composer, QObject *parent)
    : QObject(parent), d(new RichTextComposerImages::RichTextComposerImagesPrivate(composer))
{

}

RichTextComposerImages::~RichTextComposerImages()
{
    delete d;
}

void RichTextComposerImages::addImage(const QUrl &url, int width, int height)
{
    addImageHelper(url, width, height);
}

void RichTextComposerImages::addImageHelper(const QUrl &url, int width, int height)
{
    QImage image;
    if (!image.load(url.path())) {
        KMessageBox::error(
            d->composer,
            xi18nc("@info",
                   "Unable to load image <filename>%1</filename>.",
                   url.path()));
        return;
    }
    QFileInfo fi(url.path());
    QString imageName =
        fi.baseName().isEmpty() ?
        QStringLiteral("image.png") :
        QString(fi.baseName() + QLatin1String(".png"));
    addImageHelper(imageName, image, width, height);
}

void RichTextComposerImages::loadImage(const QImage &image, const QString &matchName,
                                       const QString &resourceName)
{
    QSet<int> cursorPositionsToSkip;
    QTextBlock currentBlock = d->composer->document()->begin();
    QTextBlock::iterator it;
    while (currentBlock.isValid()) {
        for (it = currentBlock.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
                if (imageFormat.isValid() && imageFormat.name() == matchName) {
                    int pos = fragment.position();
                    if (!cursorPositionsToSkip.contains(pos)) {
                        QTextCursor cursor(d->composer->document());
                        cursor.setPosition(pos);
                        cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
                        cursor.removeSelectedText();
                        d->composer->document()->addResource(QTextDocument::ImageResource,
                                                             QUrl(resourceName), QVariant(image));
                        QTextImageFormat format;
                        format.setName(resourceName);
                        if ((imageFormat.width() != 0) && (imageFormat.height() != 0)) {
                            format.setWidth(imageFormat.width());
                            format.setHeight(imageFormat.height());
                        }
                        cursor.insertImage(format);

                        // The textfragment iterator is now invalid, restart from the beginning
                        // Take care not to replace the same fragment again, or we would be in
                        // an infinite loop.
                        cursorPositionsToSkip.insert(pos);
                        it = currentBlock.begin();
                    }
                }
            }
        }
        currentBlock = currentBlock.next();
    }
}

void RichTextComposerImages::addImageHelper(const QString &imageName, const QImage &image,
        int width, int height)
{
    QString imageNameToAdd = imageName;
    QTextDocument *document = d->composer->document();

    // determine the imageNameToAdd
    int imageNumber = 1;
    while (d->mImageNames.contains(imageNameToAdd)) {
        QVariant qv = document->resource(QTextDocument::ImageResource, QUrl(imageNameToAdd));
        if (qv == image) {
            // use the same name
            break;
        }
        int firstDot = imageName.indexOf(QLatin1Char('.'));
        if (firstDot == -1) {
            imageNameToAdd = imageName + QString::number(imageNumber++);
        } else {
            imageNameToAdd = imageName.left(firstDot) + QString::number(imageNumber++) +
                             imageName.mid(firstDot);
        }
    }

    if (!d->mImageNames.contains(imageNameToAdd)) {
        document->addResource(QTextDocument::ImageResource, QUrl(imageNameToAdd), image);
        d->mImageNames << imageNameToAdd;
    }
    if (width != -1 && height != -1) {
        QTextImageFormat format;
        format.setName(imageNameToAdd);
        format.setWidth(width);
        format.setHeight(height);
        d->composer->textCursor().insertImage(format);
    } else {
        d->composer->textCursor().insertImage(imageNameToAdd);
    }
    d->composer->activateRichText();
}

ImageWithNameList RichTextComposerImages::imagesWithName() const
{
    ImageWithNameList retImages;
    QStringList seenImageNames;
    QList<QTextImageFormat> imageFormats = embeddedImageFormats();
    foreach (const QTextImageFormat &imageFormat, imageFormats) {
        if (!seenImageNames.contains(imageFormat.name())) {
            QVariant resourceData = d->composer->document()->resource(QTextDocument::ImageResource,
                                    QUrl(imageFormat.name()));
            QImage image = qvariant_cast<QImage>(resourceData);
            QString name = imageFormat.name();
            ImageWithNamePtr newImage(new ImageWithName);
            newImage->image = image;
            newImage->name = name;
            retImages.append(newImage);
            seenImageNames.append(imageFormat.name());
        }
    }
    return retImages;
}

QList< QSharedPointer<EmbeddedImage> > RichTextComposerImages::embeddedImages() const
{
    ImageWithNameList normalImages = imagesWithName();
    QList< QSharedPointer<EmbeddedImage> > retImages;
    retImages.reserve(normalImages.count());
    foreach (const ImageWithNamePtr &normalImage, normalImages) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        normalImage->image.save(&buffer, "PNG");

        qsrand(QDateTime::currentDateTime().toTime_t() + qHash(normalImage->name));
        QSharedPointer<EmbeddedImage> embeddedImage(new EmbeddedImage());
        retImages.append(embeddedImage);
        embeddedImage->image = KCodecs::Codec::codecForName("base64")->encode(buffer.buffer());
        embeddedImage->imageName = normalImage->name;
        embeddedImage->contentID = QStringLiteral("%1@KDE").arg(qrand());
    }
    return retImages;
}

QList<QTextImageFormat> RichTextComposerImages::embeddedImageFormats() const
{
    QTextDocument *doc = d->composer->document();
    QList<QTextImageFormat> retList;

    QTextBlock currentBlock = doc->begin();
    while (currentBlock.isValid()) {
        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid()) {
                QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
                if (imageFormat.isValid()) {
                    //TODO: Replace with a way to see if an image is an embedded image or a remote
                    QUrl url(imageFormat.name());
                    if (!url.isValid() || !url.scheme().startsWith(QStringLiteral("http"))) {
                        retList.append(imageFormat);
                    }
                }
            }
        }
        currentBlock = currentBlock.next();
    }
    return retList;
}

void RichTextComposerImages::insertImage(const QImage &image, const QFileInfo &fileInfo)
{
    QString imageName = fileInfo.baseName().isEmpty() ?
                        i18nc("Start of the filename for an image", "image") :
                        fileInfo.baseName();
    addImageHelper(imageName, image);
}

QByteArray RichTextComposerImages::imageNamesToContentIds(
    const QByteArray &htmlBody, const MessageComposer::ImageList &imageList)
{
    QByteArray result = htmlBody;
    if (!imageList.isEmpty()) {
        foreach (const QSharedPointer<EmbeddedImage> &image, imageList) {
            const QString newImageName = QLatin1String("cid:") + image->contentID;
            QByteArray quote("\"");
            result.replace(QByteArray(quote + image->imageName.toLocal8Bit() + quote),
                           QByteArray(quote + newImageName.toLocal8Bit() + quote));
        }
    }
    return result;
}
