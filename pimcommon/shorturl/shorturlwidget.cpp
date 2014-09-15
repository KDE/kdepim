/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "shorturl/shorturlconfiguredialog.h"
#include <KPIMUtils/ProgressIndicatorLabel>

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KToggleAction>
#include <KRun>
#include <QIcon>
#include <QNetworkConfigurationManager>

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPointer>

using namespace PimCommon;

ShortUrlWidget::ShortUrlWidget(QWidget *parent)
    : QWidget(parent),
      mShorturlServiceName(0),
      mEngine(0),
      mToggleAction(0),
      mStandalone(false)
{
    loadEngine();
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(2);
    setLayout(grid);

    QToolButton *closeBtn = new QToolButton( this );
    closeBtn->setIcon( QIcon::fromTheme( QLatin1String("dialog-close") ) );
    closeBtn->setIconSize( QSize( 16, 16 ) );
    closeBtn->setToolTip( i18n( "Close" ) );
#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
#endif
    closeBtn->setAutoRaise( true );
    connect(closeBtn, &QToolButton::clicked, this, &ShortUrlWidget::slotCloseWidget);

    grid->addWidget(closeBtn, 0, 0);

    mIndicatorLabel = new KPIMUtils::ProgressIndicatorLabel(i18n("In progress to generate short url..."));
    grid->addWidget(mIndicatorLabel, 0, 1);

    QPushButton *configure = new QPushButton(i18n("Configure..."));
    connect(configure, &QPushButton::clicked, this, &ShortUrlWidget::slotConfigure);
    grid->addWidget(configure, 0, 2);

    mShorturlServiceName = new QLabel(mEngine->shortUrlName());
    grid->addWidget(mShorturlServiceName, 1, 1);

    mConvertButton = new QPushButton(i18n("Convert"));
    grid->addWidget(mConvertButton, 1, 2);
    connect(mConvertButton, &QPushButton::clicked, this, &ShortUrlWidget::slotConvertUrl);

    mInsertShortUrl = new QPushButton(i18n("Insert Short Url"));
    connect(mInsertShortUrl, &QPushButton::clicked, this, &ShortUrlWidget::slotInsertShortUrl);
    grid->addWidget(mInsertShortUrl, 2, 2);

    QLabel *lab = new QLabel(i18n("Original url:"));
    grid->addWidget(lab, 3, 0);

    mOriginalUrl = new KLineEdit;
    mOriginalUrl->setClearButtonEnabled(true);
    mOriginalUrl->setTrapReturnKey(true);
    connect(mOriginalUrl, &KLineEdit::textChanged, this, &ShortUrlWidget::slotOriginalUrlChanged);
    connect(mOriginalUrl, &KLineEdit::returnPressed, this, &ShortUrlWidget::slotConvertUrl);
    grid->addWidget(mOriginalUrl, 3, 1);

    mCopyToClipboard = new QPushButton(i18n("Copy to clipboard"));
    connect(mCopyToClipboard, &QPushButton::clicked, this, &ShortUrlWidget::slotPasteToClipboard);
    grid->addWidget(mCopyToClipboard, 3, 2);


    lab = new QLabel(i18n("Short url:"));
    grid->addWidget(lab, 4, 0);

    mShortUrl = new QLineEdit;
    connect(mShortUrl, &QLineEdit::textChanged, this, &ShortUrlWidget::slotShortUrlChanged);
    mShortUrl->setReadOnly(true);
    grid->addWidget(mShortUrl, 4, 1);



    mOpenShortUrl = new QPushButton(i18n("Open Short Url"));
    connect(mOpenShortUrl, &QPushButton::clicked, this, &ShortUrlWidget::slotOpenShortUrl);
    grid->addWidget(mOpenShortUrl, 4, 2);


    grid->setRowStretch(5,1);
    mConvertButton->setEnabled(false);
    mCopyToClipboard->setEnabled(false);
    mInsertShortUrl->setEnabled(false);
    mOpenShortUrl->setEnabled(false);

    mNetworkConfigurationManager = new QNetworkConfigurationManager();
}

ShortUrlWidget::~ShortUrlWidget()
{
    delete mNetworkConfigurationManager;
}

void ShortUrlWidget::slotInsertShortUrl()
{
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        Q_EMIT insertShortUrl(shortUrl);
    }
}

void ShortUrlWidget::slotConfigure()
{
    QPointer<ShortUrlConfigureDialog> dlg = new ShortUrlConfigureDialog(this);
    if (dlg->exec()) {
        loadEngine();
    }
    delete dlg;
}

void ShortUrlWidget::settingsUpdated()
{
    loadEngine();
}

void ShortUrlWidget::loadEngine()
{
    delete mEngine;
    mEngine = PimCommon::ShortUrlUtils::loadEngine(this);
    if (mShorturlServiceName)
        mShorturlServiceName->setText(mEngine->shortUrlName());
    connect(mEngine, &AbstractShortUrl::shortUrlDone, this, &ShortUrlWidget::slotShortUrlDone);
    connect(mEngine, &AbstractShortUrl::shortUrlFailed, this, &ShortUrlWidget::slotShortUrlFailed);
}

void ShortUrlWidget::slotConvertUrl()
{
    if (!mNetworkConfigurationManager->isOnline()) {
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
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        QApplication::clipboard()->setText(shortUrl);
    }
}

void ShortUrlWidget::slotOriginalUrlChanged(const QString &text)
{
    mConvertButton->setEnabled(!text.isEmpty());
}

void ShortUrlWidget::slotShortUrlChanged(const QString &text)
{
    mCopyToClipboard->setEnabled(!text.isEmpty());
    mInsertShortUrl->setEnabled(!text.isEmpty());
    mOpenShortUrl->setEnabled(!text.isEmpty());
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

KToggleAction *ShortUrlWidget::toggleAction()
{
    if (!mToggleAction) {
        mToggleAction = new KToggleAction( i18n("Generate Shorten Url"), this );
        mToggleAction->setChecked(false);
    }
    return mToggleAction;
}

void ShortUrlWidget::slotOpenShortUrl()
{
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        new KRun( shortUrl, this );
    }
}
