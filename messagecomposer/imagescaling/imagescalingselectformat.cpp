/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "imagescalingselectformat.h"

#include <KLineEdit>
#include <KPushButton>
#include <KLocale>

#include <QListWidget>
#include <QHBoxLayout>

using namespace MessageComposer;

ImageScalingSelectFormatDialog::ImageScalingSelectFormatDialog(QWidget *parent)
    : KDialog(parent)
{
    QWidget *w = new QWidget( this );
    setMainWidget( w );
    setCaption( i18nc("@title:window", "Select Image Format") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    QBoxLayout *topLayout = new QVBoxLayout( w );
    topLayout->setSpacing( spacingHint() );
    mListWidget = new QListWidget;
    topLayout->addWidget(mListWidget);
    initialize();
}

ImageScalingSelectFormatDialog::~ImageScalingSelectFormatDialog()
{
}

void ImageScalingSelectFormatDialog::addImageFormat(const QString &format, const QString &mimetype)
{
    QListWidgetItem *item = new QListWidgetItem(format);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable );
    item->setData(ImageScalingSelectFormatDialog::ImageRole, mimetype);
    item->setCheckState(Qt::Unchecked);
    mListWidget->addItem(item);
}

void ImageScalingSelectFormatDialog::initialize()
{
    addImageFormat(QLatin1String("PNG"), QLatin1String("image/png"));
    addImageFormat(QLatin1String("JPEG"), QLatin1String("image/jpeg"));
    addImageFormat(QLatin1String("GIF"), QLatin1String("image/gif"));
    addImageFormat(QLatin1String("BMP"), QLatin1String("image/bmp"));
}

QString ImageScalingSelectFormatDialog::format() const
{
    const int numberOfElement(mListWidget->count());
    QString formatStr;
    for (int i=0; i < numberOfElement;++i) {
        if (mListWidget->item(i)->checkState() == Qt::Checked) {
            if (!formatStr.isEmpty()) {
                formatStr += QLatin1Char(';');
            }
            formatStr += mListWidget->item(i)->data(ImageScalingSelectFormatDialog::ImageRole).toString();
        }
    }
    return formatStr;
}

void ImageScalingSelectFormatDialog::setFormat(const QString &format)
{
    const QStringList listFormat = format.split(QLatin1Char(';'));
    const int numberOfElement(mListWidget->count());
    for (int i=0; i < numberOfElement;++i) {
        QListWidgetItem *item = mListWidget->item(i);
        if (listFormat.contains(item->data(ImageScalingSelectFormatDialog::ImageRole).toString())) {
            item->setCheckState(Qt::Checked);
        }
    }
}

ImageScalingSelectFormat::ImageScalingSelectFormat(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    mFormat = new KLineEdit;
    connect(mFormat, SIGNAL(textChanged(QString)),SIGNAL(textChanged(QString)));
    mFormat->setReadOnly(true);
    lay->addWidget(mFormat);
    mSelectFormat = new KPushButton(i18n("Select Format..."));
    connect(mSelectFormat, SIGNAL(clicked(bool)), this, SLOT(slotSelectFormat()));
    lay->addWidget(mSelectFormat);
}

ImageScalingSelectFormat::~ImageScalingSelectFormat()
{
}

void ImageScalingSelectFormat::slotSelectFormat()
{
    QPointer<ImageScalingSelectFormatDialog> dialog = new ImageScalingSelectFormatDialog(this);
    dialog->setFormat(mFormat->text());
    if (dialog->exec()) {
        mFormat->setText(dialog->format());
    }
    delete dialog;
}

void ImageScalingSelectFormat::setFormat(const QString &format)
{
    mFormat->setText(format);
}

QString ImageScalingSelectFormat::format() const
{
    return mFormat->text();
}

#include "imagescalingselectformat.moc"
