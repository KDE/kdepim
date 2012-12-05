/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "pagecolorbackgroundwidget.h"
#include "ui_pagecolorbackgroundwidget.h"

#include <QImageReader>

using namespace ComposerEditorNG;

PageColorBackgroundWidget::PageColorBackgroundWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageColorBackgroundWidget)
{
    ui->setupUi(this);
    QString imageFormat;
    QList<QByteArray> listReaderFormat = QImageReader::supportedImageFormats();
    Q_FOREACH(const QByteArray& format, listReaderFormat) {
        if(imageFormat.isEmpty()) {
            imageFormat = QString::fromLatin1("*.%1").arg(QString::fromLatin1(format));
        } else {
            imageFormat += QString::fromLatin1(" *.%1").arg(QString::fromLatin1(format));
        }
    }
    ui->backgroundImage->setFilter(imageFormat);
}

PageColorBackgroundWidget::~PageColorBackgroundWidget()
{
    delete ui;
}

QColor PageColorBackgroundWidget::pageBackgroundColor() const
{
    return ui->backgroundColor->color();
}

void PageColorBackgroundWidget::setPageBackgroundColor(const QColor &col)
{
    ui->backgroundColor->setColor(col);
}

QColor PageColorBackgroundWidget::textColor() const
{
    return ui->textColor->color();
}

void PageColorBackgroundWidget::setTextColor(const QColor &col)
{
    ui->textColor->setColor(col);
}


void PageColorBackgroundWidget::setUseDefaultColor(bool b)
{
    if(b)
        ui->defaultColor->setChecked(true);
    else
        ui->customColors->setChecked(true);
}

bool PageColorBackgroundWidget::useDefaultColor() const
{
    return ui->defaultColor->isChecked();
}

KUrl PageColorBackgroundWidget::backgroundImageUrl() const
{
    return ui->backgroundImage->url();
}

void PageColorBackgroundWidget::setBackgroundImageUrl(const KUrl& url)
{
    ui->backgroundImage->setUrl(url);
}

#include "pagecolorbackgroundwidget.moc"
