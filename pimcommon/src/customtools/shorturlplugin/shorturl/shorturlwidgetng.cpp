/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "shorturlwidgetng.h"
#include "shorturlengineplugin/shorturlengineplugin.h"
#include "shorturlengineplugin/shorturlengineinterface.h"
#include "shorturl/shorturlconfiguredialog.h"
#include "Libkdepim/ProgressIndicatorLabel"
#include "shorturlengineplugin/shorturlenginepluginmanager.h"

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KToggleAction>
#include <KSharedConfig>
#include <KRun>
#include <QIcon>
#include <QNetworkConfigurationManager>

#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QToolButton>
#include <QPointer>
#include <KActionCollection>

#include <shorturl/shorturlengineplugin/shorturlengineplugin.h>

using namespace PimCommon;

ShortUrlWidgetNg::ShortUrlWidgetNg(QWidget *parent)
    : QWidget(parent),
      mShorturlServiceName(Q_NULLPTR),
      mCurrentEngine(Q_NULLPTR)
{
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(2);
    setLayout(grid);

    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setToolTip(i18n("Close"));
#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName(i18n("Close"));
#endif
    closeBtn->setAutoRaise(true);
    connect(closeBtn, &QToolButton::clicked, this, &ShortUrlWidgetNg::slotCloseWidget);

    grid->addWidget(closeBtn, 0, 0);

    mIndicatorLabel = new KPIM::ProgressIndicatorLabel(i18n("In progress to generate short url..."));
    grid->addWidget(mIndicatorLabel, 0, 1);

    QPushButton *configure = new QPushButton(i18n("Configure..."));
    connect(configure, &QPushButton::clicked, this, &ShortUrlWidgetNg::slotConfigure);
    grid->addWidget(configure, 0, 2);

    mShorturlServiceName = new QLabel(/*mEngine->shortUrlName()*/);
    grid->addWidget(mShorturlServiceName, 1, 1);

    mConvertButton = new QPushButton(i18n("Convert"));
    grid->addWidget(mConvertButton, 1, 2);
    connect(mConvertButton, &QPushButton::clicked, this, &ShortUrlWidgetNg::slotConvertUrl);

    mInsertShortUrl = new QPushButton(i18n("Insert Short URL"));
    connect(mInsertShortUrl, &QPushButton::clicked, this, &ShortUrlWidgetNg::slotInsertShortUrl);
    grid->addWidget(mInsertShortUrl, 2, 2);

    QLabel *lab = new QLabel(i18n("Original url:"));
    grid->addWidget(lab, 3, 0);

    mOriginalUrl = new KLineEdit;
    mOriginalUrl->setClearButtonEnabled(true);
    mOriginalUrl->setTrapReturnKey(true);
    connect(mOriginalUrl, &KLineEdit::textChanged, this, &ShortUrlWidgetNg::slotOriginalUrlChanged);
    connect(mOriginalUrl, &KLineEdit::returnPressed, this, &ShortUrlWidgetNg::slotConvertUrl);
    grid->addWidget(mOriginalUrl, 3, 1);

    mCopyToClipboard = new QPushButton(i18n("Copy to clipboard"));
    connect(mCopyToClipboard, &QPushButton::clicked, this, &ShortUrlWidgetNg::slotPasteToClipboard);
    grid->addWidget(mCopyToClipboard, 3, 2);

    lab = new QLabel(i18n("Short url:"));
    grid->addWidget(lab, 4, 0);

    mShortUrl = new QLineEdit;
    connect(mShortUrl, &QLineEdit::textChanged, this, &ShortUrlWidgetNg::slotShortUrlChanged);
    mShortUrl->setReadOnly(true);
    grid->addWidget(mShortUrl, 4, 1);

    mOpenShortUrl = new QPushButton(i18n("Open Short URL"));
    connect(mOpenShortUrl, &QPushButton::clicked, this, &ShortUrlWidgetNg::slotOpenShortUrl);
    grid->addWidget(mOpenShortUrl, 4, 2);

    grid->setRowStretch(5, 1);
    mConvertButton->setEnabled(false);
    mCopyToClipboard->setEnabled(false);
    mInsertShortUrl->setEnabled(false);
    mOpenShortUrl->setEnabled(false);

    mNetworkConfigurationManager = new QNetworkConfigurationManager();
    initializePlugins();
    loadEngine();
}

ShortUrlWidgetNg::~ShortUrlWidgetNg()
{
    delete mNetworkConfigurationManager;
}

void ShortUrlWidgetNg::initializePlugins()
{
    const QVector<PimCommon::ShortUrlEnginePlugin *>  lstPlugin = PimCommon::ShortUrlEnginePluginManager::self()->pluginsList();
    Q_FOREACH (PimCommon::ShortUrlEnginePlugin *plugin, lstPlugin) {
        PimCommon::ShortUrlEngineInterface *interface = plugin->createInterface(this);
        mLstInterface.insert(interface->engineName(), interface);
    }
}

void ShortUrlWidgetNg::slotInsertShortUrl()
{
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        Q_EMIT insertText(shortUrl);
    }
}

void ShortUrlWidgetNg::slotConfigure()
{
    QPointer<ShortUrlConfigureDialog> dlg = new ShortUrlConfigureDialog(this);
    if (dlg->exec()) {
        loadEngine();
    }
    delete dlg;
}

void ShortUrlWidgetNg::settingsUpdated()
{
    loadEngine();
}

void ShortUrlWidgetNg::loadEngine()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "ShortUrl");
    const QString engineName = grp.readEntry("EngineName");
    mCurrentEngine = mLstInterface.value(engineName);
    if (!mCurrentEngine) {
        mCurrentEngine = mLstInterface.cbegin().value();
    }
    mShorturlServiceName->setText(mCurrentEngine->engineName()); //TODO use correct name

    if (mCurrentEngine) {
        connect(mCurrentEngine, &ShortUrlEngineInterface::shortUrlGenerated, this, &ShortUrlWidgetNg::slotShortUrlDone);
        connect(mCurrentEngine, &ShortUrlEngineInterface::shortUrlFailed, this, &ShortUrlWidgetNg::slotShortUrlFailed);
    }
}

void ShortUrlWidgetNg::slotConvertUrl()
{
    if (!mCurrentEngine) {
        return;
    }
    if (!mNetworkConfigurationManager->isOnline()) {
        KMessageBox::information(this, i18n("No network connection detected, we cannot shorten url."), i18n("No network"));
        return;
    }
    if (mOriginalUrl->text().isEmpty()) {
        return;
    }
    mIndicatorLabel->start();
    mCurrentEngine->setShortUrl(mOriginalUrl->text());
    mShortUrl->clear();
    mCurrentEngine->generateShortUrl();
}

void ShortUrlWidgetNg::slotPasteToClipboard()
{
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        QApplication::clipboard()->setText(shortUrl);
    }
}

void ShortUrlWidgetNg::slotOriginalUrlChanged(const QString &text)
{
    mConvertButton->setEnabled(!text.isEmpty());
}

void ShortUrlWidgetNg::slotShortUrlChanged(const QString &text)
{
    mCopyToClipboard->setEnabled(!text.isEmpty());
    mInsertShortUrl->setEnabled(!text.isEmpty());
    mOpenShortUrl->setEnabled(!text.isEmpty());
}

void ShortUrlWidgetNg::slotShortUrlDone(const QString &url)
{
    mShortUrl->setText(url);
    mIndicatorLabel->stop();
}

void ShortUrlWidgetNg::slotShortUrlFailed(const QString &errMsg)
{
    KMessageBox::error(this, i18n("An error occurs: \"%1\"", errMsg));
    mIndicatorLabel->stop();
}

void ShortUrlWidgetNg::slotCloseWidget()
{
    mOriginalUrl->clear();
    mShortUrl->clear();
    mIndicatorLabel->stop();


    Q_EMIT toolsWasClosed();
}


void ShortUrlWidgetNg::slotOpenShortUrl()
{
    const QString shortUrl = mShortUrl->text();
    if (!shortUrl.isEmpty()) {
        new KRun(QUrl(shortUrl), this);
    }
}
