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

#include <KPIMUtils/ProgressIndicatorLabel>

#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QToolButton>

using namespace PimCommon;

ShortUrlWidget::ShortUrlWidget(QWidget *parent)
    : QWidget(parent),
      mEngine(0),
      mStandalone(false)
{
    loadEngine();
    QGridLayout *grid = new QGridLayout;
    setLayout(grid);

    QToolButton *closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( QLatin1String("dialog-close") ) );
    closeBtn->setIconSize( QSize( 16, 16 ) );
    closeBtn->setToolTip( i18n( "Close" ) );
#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
#endif
    closeBtn->setAutoRaise( true );
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseWidget()) );

    grid->addWidget(closeBtn, 0, 0);
    mIndicatorLabel = new KPIMUtils::ProgressIndicatorLabel(i18n("In progress to generate short url..."));
    grid->addWidget(mIndicatorLabel, 0, 1, 1, 2);


    QLabel *lab = new QLabel(i18n("Original url:"));
    grid->addWidget(lab, 1, 0);

    mOriginalUrl = new KLineEdit;
    mOriginalUrl->setClearButtonShown(true);
    mOriginalUrl->setTrapReturnKey(true);
    connect(mOriginalUrl, SIGNAL(textChanged(QString)), this, SLOT(slotOriginalUrlChanged(QString)));
    connect(mOriginalUrl, SIGNAL(returnPressed(QString)), this, SLOT(slotConvertUrl()));
    grid->addWidget(mOriginalUrl, 1, 1);

    mConvertButton = new QPushButton(i18n("Convert"));
    grid->addWidget(mConvertButton, 1, 2);
    connect(mConvertButton, SIGNAL(clicked()), this, SLOT(slotConvertUrl()));

    lab = new QLabel(i18n("Short url:"));
    grid->addWidget(lab, 2, 0);

    mShortUrl = new KLineEdit;
    connect(mShortUrl, SIGNAL(textChanged(QString)), this, SLOT(slotShortUrlChanged(QString)));
    mShortUrl->setReadOnly(true);
    grid->addWidget(mShortUrl, 2, 1);

    mCopyToClipboard = new QPushButton(i18n("Copy to clipboard"));
    connect(mCopyToClipboard, SIGNAL(clicked()), this, SLOT(slotPasteToClipboard()));
    grid->addWidget(mCopyToClipboard, 2, 2);
    mConvertButton->setEnabled(false);
    mCopyToClipboard->setEnabled(false);

    connect ( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
              this, SLOT(slotSystemNetworkStatusChanged(Solid::Networking::Status)) );
    Solid::Networking::Status networkStatus = Solid::Networking::status();
    if ( ( networkStatus == Solid::Networking::Unconnected ) ||
         ( networkStatus == Solid::Networking::Disconnecting ) ||
         ( networkStatus == Solid::Networking::Connecting ))
        mNetworkUp = false;
    else
        mNetworkUp = true;
}

ShortUrlWidget::~ShortUrlWidget()
{
}

void ShortUrlWidget::settingsUpdated()
{
    loadEngine();
}

void ShortUrlWidget::loadEngine()
{
    delete mEngine;
    mEngine = PimCommon::ShortUrlUtils::loadEngine(this);
    connect(mEngine, SIGNAL(shortUrlDone(QString)), this, SLOT(slotShortUrlDone(QString)));
    connect(mEngine, SIGNAL(shortUrlFailed(QString)), this, SLOT(slotShortUrlFailed(QString)));
}

void ShortUrlWidget::slotConvertUrl()
{
    if (!mNetworkUp) {
        KMessageBox::information(this, i18n("No network connection detected, we cannot shorten url."), i18n("No network"));
        return;
    }
    if (mOriginalUrl->text().isEmpty())
        return;
    mIndicatorLabel->start();
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
    mIndicatorLabel->stop();
}

void ShortUrlWidget::slotShortUrlFailed(const QString &errMsg)
{
    KMessageBox::error(this, i18n("An error occurs: \"%1\"", errMsg));
    mIndicatorLabel->stop();
}

void ShortUrlWidget::slotSystemNetworkStatusChanged( Solid::Networking::Status status )
{
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
        mNetworkUp = true;
    } else {
        mNetworkUp = false;
    }
}

void ShortUrlWidget::slotCloseWidget()
{
    mOriginalUrl->clear();
    mShortUrl->clear();
    mIndicatorLabel->stop();

    if (mStandalone)
        hide();

    Q_EMIT shortUrlWasClosed();
}

void ShortUrlWidget::setStandalone(bool b)
{
    mStandalone = b;
}

