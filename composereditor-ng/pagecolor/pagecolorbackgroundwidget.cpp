/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include <KImageIO>
#include <KFileDialog>

using namespace ComposerEditorNG;

PageColorBackgroundWidget::PageColorBackgroundWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageColorBackgroundWidget)
{
    ui->setupUi(this);

    const QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Reading );
    ui->backgroundImage->fileDialog()->setFilter(mimetypes.join( QLatin1String( " " ) ));
    ui->groupBox->setEnabled(false);
    connect(ui->customColors,SIGNAL(toggled(bool)),ui->groupBox,SLOT(setEnabled(bool)));
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
    if (b)
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

QColor PageColorBackgroundWidget::linkColor() const
{
    return ui->linkColor->color();
}

void PageColorBackgroundWidget::setLinkColor(const QColor &col)
{
    ui->linkColor->setColor(col);
}

QColor PageColorBackgroundWidget::activeLinkColor() const
{
    return ui->activeLinkColor->color();
}

void PageColorBackgroundWidget::setActiveLinkColor(const QColor &col)
{
    ui->activeLinkColor->setColor(col);
}

QColor PageColorBackgroundWidget::visitedLinkColor() const
{
    return ui->visitedLinkColor->color();
}

void PageColorBackgroundWidget::setVisitedLinkColor(const QColor &col)
{
    ui->visitedLinkColor->setColor(col);
}

#include "pagecolorbackgroundwidget.moc"
