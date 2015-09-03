/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "translatorview.h"

#include <QHBoxLayout>
#include <KLocalizedString>
#include <KToggleAction>

using namespace PimCommon;
TranslatorView::TranslatorView(QWidget *parent)
    : PimCommon::CustomToolsViewInterface(parent),
      mAction(Q_NULLPTR)
{
    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    mTranslatorWidget = new PimCommon::TranslatorWidget(this);
    connect(mTranslatorWidget, &PimCommon::TranslatorWidget::toolsWasClosed, this, &TranslatorView::toolsWasClosed);

    layout->addWidget(mTranslatorWidget);
    createAction();
}


TranslatorView::~TranslatorView()
{

}

KToggleAction *TranslatorView::action() const
{
    return mAction;
}

void TranslatorView::slotActivateTranslator(bool state)
{
    if (state) {
        mTranslatorWidget->show();
        Q_EMIT activateView(this);
    } else {
        mTranslatorWidget->hide();
        Q_EMIT activateView(Q_NULLPTR);
    }
}

void TranslatorView::createAction()
{
    mAction = new KToggleAction(i18n("&Translator"), this);
    connect(mAction, &KToggleAction::triggered, this, &TranslatorView::slotActivateTranslator);
    mAction->setChecked(false);
}
