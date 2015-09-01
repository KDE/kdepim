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

#include "customtoolswidget.h"

#include "pimcommon/shorturl/shorturlwidget.h"
#include "pimcommon/translator/translatorwidget.h"

#include <KToggleAction>

#include <QHBoxLayout>
#include <QStackedWidget>
#include "pimcommon_debug.h"

using namespace PimCommon;
class PimCommon::CustomToolsWidgetPrivate
{
public:
    CustomToolsWidgetPrivate()
        : mStackedWidget(Q_NULLPTR),
          mShortUrlWidget(Q_NULLPTR),
          mTranslatorWidget(Q_NULLPTR)
    {

    }
    QStackedWidget *mStackedWidget;
    ShortUrlWidget *mShortUrlWidget;
    TranslatorWidget *mTranslatorWidget;
};

CustomToolsWidget::CustomToolsWidget(QWidget *parent, KActionCollection *ac)
    : QWidget(parent),
      d(new PimCommon::CustomToolsWidgetPrivate)
{
    QHBoxLayout *lay = new QHBoxLayout;
    d->mStackedWidget = new QStackedWidget;
    lay->addWidget(d->mStackedWidget);
    setLayout(lay);

    d->mShortUrlWidget = new ShortUrlWidget;
    d->mTranslatorWidget = new TranslatorWidget;
    d->mShortUrlWidget->setStandalone(false);
    d->mTranslatorWidget->setStandalone(false);
    d->mStackedWidget->addWidget(d->mShortUrlWidget);
    d->mStackedWidget->addWidget(d->mTranslatorWidget);

    d->mShortUrlWidget->createAction(ac);
    d->mTranslatorWidget->createAction(ac);

    connect(d->mShortUrlWidget, &ShortUrlWidget::shortUrlWasClosed, this, &CustomToolsWidget::slotHideTools);
    connect(d->mShortUrlWidget->toggleAction(), &KToggleAction::triggered, this, &CustomToolsWidget::slotVisibleShortUrlTools);

    connect(d->mTranslatorWidget, &TranslatorWidget::translatorWasClosed, this, &CustomToolsWidget::slotHideTools);
    connect(d->mTranslatorWidget->toggleAction(), &KToggleAction::triggered, this, &CustomToolsWidget::slotVisibleTranslatorTools);

    connect(d->mShortUrlWidget, &ShortUrlWidget::shortUrlWasClosed, this, &CustomToolsWidget::shortUrlWasClosed);
    connect(d->mShortUrlWidget, &ShortUrlWidget::insertShortUrl, this, &CustomToolsWidget::insertShortUrl);

    connect(d->mTranslatorWidget, &TranslatorWidget::translatorWasClosed, this, &CustomToolsWidget::translatorWasClosed);

    d->mStackedWidget->setCurrentWidget(d->mTranslatorWidget);
    hide();
}

CustomToolsWidget::~CustomToolsWidget()
{
    delete d;
}

void CustomToolsWidget::slotVisibleTranslatorTools(bool b)
{
    if (b) {
        switchToTool(PimCommon::CustomToolsWidget::TranslatorTool);
    } else {
        customToolWasClosed();
    }
    setVisible(b);
}

void CustomToolsWidget::slotVisibleShortUrlTools(bool b)
{
    if (b) {
        switchToTool(PimCommon::CustomToolsWidget::ShortUrlTool);
    } else {
        customToolWasClosed();
    }
    setVisible(b);
}

void CustomToolsWidget::customToolWasClosed()
{
    d->mShortUrlWidget->toggleAction()->setChecked(false);
    d->mTranslatorWidget->toggleAction()->setChecked(false);
}

ShortUrlWidget *CustomToolsWidget::shortUrlWidget() const
{
    return d->mShortUrlWidget;
}

TranslatorWidget *CustomToolsWidget::translatorWidget() const
{
    return d->mTranslatorWidget;
}

KToggleAction *CustomToolsWidget::action(CustomToolsWidget::ToolType type)
{
    KToggleAction *act = Q_NULLPTR;
    switch (type) {
    case TranslatorTool:
        act = d->mTranslatorWidget->toggleAction();
        break;
    case ShortUrlTool:
        act = d->mShortUrlWidget->toggleAction();
        break;
    default:
        qCDebug(PIMCOMMON_LOG) << " type unknown :" << type;
        break;
    }
    return act;
}

void CustomToolsWidget::switchToTool(CustomToolsWidget::ToolType type)
{
    switch (type) {
    case TranslatorTool:
        d->mStackedWidget->setCurrentWidget(d->mTranslatorWidget);
        d->mShortUrlWidget->toggleAction()->setChecked(false);
        break;
    case ShortUrlTool:
        d->mStackedWidget->setCurrentWidget(d->mShortUrlWidget);
        d->mTranslatorWidget->toggleAction()->setChecked(false);
        break;
    default:
        qCDebug(PIMCOMMON_LOG) << " type unknown :" << type;
        break;
    }
    Q_EMIT toolSwitched(type);
}

CustomToolsWidget::ToolType CustomToolsWidget::toolType() const
{
    if (d->mStackedWidget->currentWidget() == d->mTranslatorWidget) {
        return TranslatorTool;
    } else if (d->mStackedWidget->currentWidget() == d->mShortUrlWidget) {
        return ShortUrlTool;
    } else {
        qCDebug(PIMCOMMON_LOG) << " unknown tool";
        return TranslatorTool;
    }
}

void CustomToolsWidget::slotHideTools()
{
    customToolWasClosed();
    hide();
}

#include "moc_customtoolswidget.cpp"
