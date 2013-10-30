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

#include "shorturlwidget.h"

#include <KLineEdit>
#include <KLocale>

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

using namespace PimCommon;

ShortUrlWidget::ShortUrlWidget(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout;

    QLabel *lab = new QLabel(i18n("Original url:"));
    grid->addWidget(lab, 0, 0);

    mOriginalUrl = new KLineEdit;
    connect(mOriginalUrl, SIGNAL(textChanged(QString)), this, SLOT(slotOriginalUrlChanged(QString)));
    grid->addWidget(mOriginalUrl, 0, 1);

    mConvertButton = new QPushButton(i18n("Convert"));
    grid->addWidget(mConvertButton, 0, 2);
    connect(mConvertButton, SIGNAL(clicked()), this, SLOT(slotConvertUrl()));

    lab = new QLabel(i18n("Short url:"));
    grid->addWidget(lab, 1, 0);

    mShortUrl = new KLineEdit;
    connect(mShortUrl, SIGNAL(textChanged(QString)), this, SLOT(slotShortUrlChanged(QString)));
    mShortUrl->setReadOnly(true);
    grid->addWidget(mShortUrl, 1, 1);

    mPasteToClipboard = new QPushButton(i18n("Paste to clipboard"));
    connect(mPasteToClipboard, SIGNAL(clicked()), this, SLOT(slotPasteToClipboard()));
    setLayout(grid);
    mConvertButton->setEnabled(false);
    mPasteToClipboard->setEnabled(false);
}

ShortUrlWidget::~ShortUrlWidget()
{
}

void ShortUrlWidget::slotConvertUrl()
{
    //TODO
}

void ShortUrlWidget::slotPasteToClipboard()
{
    //TODO
}

void ShortUrlWidget::slotOriginalUrlChanged(const QString &text)
{
    mConvertButton->setEnabled(!text.isEmpty());
}

void ShortUrlWidget::slotShortUrlChanged(const QString &text)
{
    mPasteToClipboard->setEnabled(!text.isEmpty());
}

#include "shorturlwidget.moc"
