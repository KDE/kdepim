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
    initialize();
}

ImageScalingSelectFormatDialog::~ImageScalingSelectFormatDialog()
{
}

void ImageScalingSelectFormatDialog::initialize()
{
    QListWidgetItem *item = new QListWidgetItem(QLatin1String("PNG"));
    item->setData(ImageScalingSelectFormatDialog::ImageRole, QLatin1String("image/png"));
    mListWidget->addItem(item);

    item = new QListWidgetItem(QLatin1String("JPEG"));
    item->setData(ImageScalingSelectFormatDialog::ImageRole, QLatin1String("image/jpeg"));
    mListWidget->addItem(item);

    item = new QListWidgetItem(QLatin1String("GIF"));
    item->setData(ImageScalingSelectFormatDialog::ImageRole, QLatin1String("image/gif"));
    mListWidget->addItem(item);
}

QString ImageScalingSelectFormatDialog::format() const
{
    //TODO
    return QString();
}

void ImageScalingSelectFormatDialog::setFormat(const QString &format)
{
    mListWidget->clear();
    const QStringList listFormat = format.split(QLatin1Char(';'));
    Q_FOREACH (const QString &str, listFormat) {

    }

    //TODO
}

ImageScalingSelectFormat::ImageScalingSelectFormat(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mFormat = new KLineEdit;
    mFormat->setReadOnly(true);
    lay->addWidget(mFormat);
    mSelectFormat = new KPushButton(i18n("..."));
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
