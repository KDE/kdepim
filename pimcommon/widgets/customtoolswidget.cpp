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

#include "customtoolswidget.h"

#include "pimcommon/shorturl/shorturlwidget.h"
#include "pimcommon/translator/translatorwidget.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QDebug>

using namespace PimCommon;

CustomToolsWidget::CustomToolsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mStackedWidget = new QStackedWidget;
    lay->addWidget(mStackedWidget);
    setLayout(lay);

    mShortUrlWidget = new ShortUrlWidget;
    mTranslatorWidget = new TranslatorWidget;
    mStackedWidget->addWidget(mShortUrlWidget);
    mStackedWidget->addWidget(mTranslatorWidget);
    connect(mShortUrlWidget, SIGNAL(shortUrlWasClosed()), this, SLOT(slotHideTools()));
    connect(mTranslatorWidget, SIGNAL(translatorWasClosed()), this, SLOT(slotHideTools()));
    mStackedWidget->setCurrentWidget(mTranslatorWidget);
}

CustomToolsWidget::~CustomToolsWidget()
{

}

void CustomToolsWidget::switchToTool(CustomToolsWidget::ToolType type)
{
    switch (type) {
    case TranslatorTool:
        mStackedWidget->setCurrentWidget(mTranslatorWidget);
        break;
    case ShortUrlTool:
        mStackedWidget->setCurrentWidget(mShortUrlWidget);
        break;
    default:
        qDebug()<<" type unknown :"<<type;
        break;
    }
}

CustomToolsWidget::ToolType CustomToolsWidget::toolType() const
{
    if (mStackedWidget->currentWidget() == mTranslatorWidget)
        return TranslatorTool;
    else if (mStackedWidget->currentWidget() == mShortUrlWidget)
        return ShortUrlTool;
    else {
        qDebug()<<" unknow tool";
        return TranslatorTool;
    }
}

void CustomToolsWidget::slotHideTools()
{
    hide();
}

#include "moc_customtoolswidget.cpp"
