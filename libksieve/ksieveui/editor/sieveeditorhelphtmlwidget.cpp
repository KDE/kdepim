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

#include "sieveeditorhelphtmlwidget.h"
#include "sieveeditorloadprogressindicator.h"
#include "sieveeditorwebview.h"
#include <QVBoxLayout>

using namespace KSieveUi;

SieveEditorHelpHtmlWidget::SieveEditorHelpHtmlWidget(QWidget *parent)
    : QWidget(parent)
{
    mProgressIndicator = new SieveEditorLoadProgressIndicator(this);
    connect(mProgressIndicator, SIGNAL(pixmapChanged(QPixmap)), this, SLOT(slotPixmapChanged(QPixmap)));
    connect(mProgressIndicator, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));
    mWebView = new SieveEditorWebView;
    connect(mWebView, SIGNAL(titleChanged(QString)), this, SLOT(slotTitleChanged(QString)));
    connect(mWebView, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(mWebView, SIGNAL(loadFinished(bool)), this, SLOT(slotFinished(bool)));
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(mWebView);
    setLayout(lay);
}

SieveEditorHelpHtmlWidget::~SieveEditorHelpHtmlWidget()
{

}

void SieveEditorHelpHtmlWidget::slotLoadFinished(bool success)
{
    Q_EMIT loadFinished(this, success);
}

void SieveEditorHelpHtmlWidget::slotPixmapChanged(const QPixmap &pixmap)
{
    Q_EMIT progressIndicatorPixmapChanged(this, pixmap);
}

void SieveEditorHelpHtmlWidget::slotFinished(bool b)
{
    mProgressIndicator->stopAnimation(b);
}

void SieveEditorHelpHtmlWidget::slotLoadStarted()
{
    mProgressIndicator->startAnimation();
}

void SieveEditorHelpHtmlWidget::slotTitleChanged(const QString &)
{
    Q_EMIT titleChanged(this, mVariableName);
}

void SieveEditorHelpHtmlWidget::setHelp(const QString &variableName, const QString &url)
{
    mVariableName = variableName;
    mWebView->setUrl(url);
}

QString SieveEditorHelpHtmlWidget::variableName() const
{
    return mVariableName;
}
