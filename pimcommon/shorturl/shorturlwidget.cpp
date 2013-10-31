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
#include "shorturlutils.h"
#include "abstractshorturl.h"

#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>

using namespace PimCommon;

ShortUrlWidget::ShortUrlWidget(QWidget *parent)
    : QWidget(parent)
{
    loadEngine();
    QGridLayout *grid = new QGridLayout;

    QLabel *lab = new QLabel(i18n("Original url:"));
    grid->addWidget(lab, 0, 0);

    mOriginalUrl = new KLineEdit;
    mOriginalUrl->setClearButtonShown(true);
    mOriginalUrl->setTrapReturnKey(true);
    connect(mOriginalUrl, SIGNAL(textChanged(QString)), this, SLOT(slotOriginalUrlChanged(QString)));
    connect(mOriginalUrl, SIGNAL(returnPressed(QString)), this, SLOT(slotConvertUrl()));
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

    mCopyToClipboard = new QPushButton(i18n("Copy to clipboard"));
    connect(mCopyToClipboard, SIGNAL(clicked()), this, SLOT(slotPasteToClipboard()));
    grid->addWidget(mCopyToClipboard, 1, 2);
    setLayout(grid);
    mConvertButton->setEnabled(false);
    mCopyToClipboard->setEnabled(false);
}

ShortUrlWidget::~ShortUrlWidget()
{
}

void ShortUrlWidget::loadEngine()
{
    mEngine = PimCommon::ShortUrlUtils::loadEngine(this);
    connect(mEngine, SIGNAL(shortUrlDone(QString)), this, SLOT(slotShortUrlDone(QString)));
    connect(mEngine, SIGNAL(shortUrlFailed(QString)), this, SLOT(slotShortUrlFailed(QString)));
}

void ShortUrlWidget::slotConvertUrl()
{
    if (mOriginalUrl->text().isEmpty())
        return;
    mEngine->shortUrl(mOriginalUrl->text());
    mShortUrl->clear();
    mEngine->start();
}

void ShortUrlWidget::slotPasteToClipboard()
{
    if (!mShortUrl->text().isEmpty()) {
        QApplication::clipboard()->setText(mShortUrl->text());
    }
}

void ShortUrlWidget::slotOriginalUrlChanged(const QString &text)
{
    mConvertButton->setEnabled(!text.isEmpty());
}

void ShortUrlWidget::slotShortUrlChanged(const QString &text)
{
    mCopyToClipboard->setEnabled(!text.isEmpty());
}

void ShortUrlWidget::slotShortUrlDone(const QString &url)
{
    mShortUrl->setText(url);
}

void ShortUrlWidget::slotShortUrlFailed(const QString &errMsg)
{
    KMessageBox::error(this, i18n("An error occurs: \"%1\"", errMsg));
}

#include "shorturlwidget.moc"
