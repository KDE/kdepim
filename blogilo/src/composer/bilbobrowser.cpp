/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "bilbobrowser.h"

#include <QWebEngineView>
#include <QProgressBar>
#include <qstatusbar.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <QPushButton>
#include "blogilo_debug.h"

#include "stylegetter.h"
#include "global.h"
#include "settings.h"
#include <klocalizedstring.h>
#include <dbman.h>
#include <bilboblog.h>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QTimer>

BilboBrowser::BilboBrowser(QWidget *parent)
    : QWidget(parent)
{
    mWebView = new QWebEngineView(this);

    createUi(parent);
    connect(mWebView, &QWebEngineView::loadProgress,
            browserProgress, &QProgressBar::setValue);
    connect(mWebView, &QWebEngineView::loadFinished, this, &BilboBrowser::slotCompleted);
    //connect(mWebView, &QWebView::statusBarMessage, this,
    //        &BilboBrowser::slotSetStatusBarText);

    //TODO
}

BilboBrowser::~BilboBrowser()
{
    qCDebug(BLOGILO_LOG);
}

void BilboBrowser::createUi(QWidget *parent)
{
    btnGetStyle = new QPushButton(this);
    btnGetStyle->setText(i18n("Get blog style"));
    connect(btnGetStyle, &QPushButton::clicked, this, &BilboBrowser::slotGetBlogStyle);

    viewInBlogStyle = new QCheckBox(i18n("View post in the blog style"), this);
    viewInBlogStyle->setChecked(Settings::previewInBlogStyle());
    connect(viewInBlogStyle, &QAbstractButton::toggled, this, &BilboBrowser::slotViewModeChanged);

    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20,
            QSizePolicy::Expanding, QSizePolicy::Minimum);
    KSeparator *separator = new KSeparator(this);

    QVBoxLayout *vlayout = new QVBoxLayout(parent);
    QHBoxLayout *hlayout = new QHBoxLayout();

    hlayout->addWidget(viewInBlogStyle);
    hlayout->addItem(horizontalSpacer);
    hlayout->addWidget(btnGetStyle);

    vlayout->addLayout(hlayout);
    vlayout->addWidget(separator);
    vlayout->addWidget(mWebView);

    browserProgress = new QProgressBar(this);
    browserProgress->setFixedSize(120, 17);

    browserStatus = new QStatusBar(this);
    browserStatus->setFixedHeight(20);
    browserStatus->addPermanentWidget(browserProgress);
    vlayout->addWidget(browserStatus);
}

void BilboBrowser::setHtml(const QString &title, const QString &content)
{
    currentTitle = title;
    currentContent = content;

    if (browserProgress->isHidden()) {
        browserProgress->show();
    }
    browserProgress->reset();
    browserStatus->showMessage(i18n("loading page items..."));
    if (__currentBlogId > -1 && viewInBlogStyle->isChecked()) {
        mWebView->setHtml(StyleGetter::styledHtml(__currentBlogId, title, content),
                          DBMan::self()->blog(__currentBlogId)->url());
    } else {
        mWebView->setHtml(QLatin1String("<html><body><h2 align='center'>") + title + QLatin1String("</h2>") + content + QLatin1String("</html>"));
    }
}

void BilboBrowser::stop()
{
    mWebView->stop();
}

void BilboBrowser::slotGetBlogStyle()
{
    stop();
    if (__currentBlogId < 0) {
        KMessageBox::information(this,
                                 i18n("Please select a blog, then try again."),
                                 i18n("Select a blog"));
        return;
    }

    browserStatus->showMessage(i18n("Fetching blog style from the web..."));
    if (browserProgress->isHidden()) {
        browserProgress->show();
    }
    browserProgress->reset();

    StyleGetter *styleGetter = new StyleGetter(__currentBlogId, this);
    connect(styleGetter, &StyleGetter::sigGetStyleProgress, browserProgress,
            &QProgressBar::setValue);
    connect(styleGetter, &StyleGetter::sigStyleFetched, this, &BilboBrowser::slotSetBlogStyle);
}

void BilboBrowser::slotSetBlogStyle()
{
    browserStatus->showMessage(i18n("Blog style fetched."), 2000);
    Q_EMIT sigSetBlogStyle();

    if (qobject_cast< StyleGetter * >(sender())) {
        sender()->deleteLater();
    }
}

void BilboBrowser::slotCompleted(bool ok)
{
    QTimer::singleShot(1500, browserProgress, &QWidget::hide);
    if (!ok) {
        browserStatus->showMessage(i18n("An error occurred in the latest transaction."), 5000);
    }
}

void BilboBrowser::slotSetStatusBarText(const QString &text)
{
    QString statusText = text;
    statusText.remove(QStringLiteral("<qt>"));
    browserStatus->showMessage(statusText);
}

void BilboBrowser::slotViewModeChanged()
{
    stop();
    setHtml(currentTitle, currentContent);
}

