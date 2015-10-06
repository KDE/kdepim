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

#include "shorturlview.h"
#include <KToggleAction>
#include <KLocalizedString>
#include <QHBoxLayout>
#include <kactioncollection.h>
#include <shorturl/shorturlwidgetng.h>

using namespace PimCommon;

ShorturlView::ShorturlView(KActionCollection *ac, QWidget *parent)
    : PimCommon::CustomToolsViewInterface(parent),
      mAction(Q_NULLPTR),
      mShorturl(Q_NULLPTR)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    createAction(ac);
    mShorturl = new PimCommon::ShortUrlWidgetNg(this);
    connect(mShorturl, &PimCommon::ShortUrlWidgetNg::toolsWasClosed, this, &ShorturlView::toolsWasClosed);
    connect(mShorturl, &PimCommon::ShortUrlWidgetNg::insertText, this, &ShorturlView::insertText);

    layout->addWidget(mShorturl);
    setLayout(layout);
}

ShorturlView::~ShorturlView()
{

}

KToggleAction *ShorturlView::action() const
{
    return mAction;
}

void ShorturlView::createAction(KActionCollection *ac)
{
    mAction = new KToggleAction(i18n("Generate Shortened URL"), this);
    connect(mAction, &KToggleAction::triggered, this, &ShorturlView::slotActivateShorturl);
    if (ac) {
        ac->addAction(QStringLiteral("shorten_url"), mAction);
    }
    mAction->setChecked(false);
}

void ShorturlView::slotActivateShorturl(bool state)
{
    if (state) {
        mShorturl->show();
        Q_EMIT activateView(this);
    } else {
        mShorturl->hide();
        Q_EMIT activateView(Q_NULLPTR);
    }
}
