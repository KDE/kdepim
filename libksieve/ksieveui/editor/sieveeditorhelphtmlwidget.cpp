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

#include "sieveeditorhelphtmlwidget.h"

#include <QWebView>
#include <QVBoxLayout>

using namespace KSieveUi;

SieveEditorHelpHtmlWidget::SieveEditorHelpHtmlWidget(QWidget *parent)
    : QWidget(parent)
{
    mWebView = new QWebView;
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(mWebView);
    setLayout(lay);
}

SieveEditorHelpHtmlWidget::~SieveEditorHelpHtmlWidget()
{

}

void SieveEditorHelpHtmlWidget::setHelp(const QString &url)
{
    mWebView->setUrl(url);
    connect(mWebView, SIGNAL(titleChanged(QString)), this, SLOT(slotTitleChanged(QString)));
}

void SieveEditorHelpHtmlWidget::slotTitleChanged(const QString &title)
{
    Q_EMIT titleChanged(this, title);
}
