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

#include "shorturlplugin.h"
#include "shorturl/shorturlwidget.h"
#include <KLocalizedString>
#include <KToggleAction>

using namespace PimCommon;

ShorturlPlugin::ShorturlPlugin(QObject *parent)
    : PimCommon::CustomToolsPlugin(parent),
      mAction(Q_NULLPTR)
{

}

ShorturlPlugin::~ShorturlPlugin()
{

}

void ShorturlPlugin::createAction()
{
    mAction = new KToggleAction(i18n("Generate Shorten Url"), this);
    connect(mAction, &KToggleAction::toggled, this, &ShorturlPlugin::slotActivateShorturl);
    mAction->setChecked(false);
}

KToggleAction *ShorturlPlugin::action() const
{
    return mAction;
}

QWidget *ShorturlPlugin::createView(QWidget *parent)
{
    mShortUrlWidget = new PimCommon::ShortUrlWidget(parent);
    connect(mShortUrlWidget, &PimCommon::ShortUrlWidget::toolsWasClosed, this, &ShorturlPlugin::toolsWasClosed);
    connect(mShortUrlWidget, &ShortUrlWidget::insertText, this, &ShorturlPlugin::insertText);
    return mShortUrlWidget;
}

QString ShorturlPlugin::customToolName() const
{
    return i18n("Translator");
}

void ShorturlPlugin::setShortcut(KActionCollection *ac)
{
    Q_UNUSED(ac);
}

void ShorturlPlugin::slotActivateShorturl(bool b)
{
    Q_EMIT activateTool(b ? mShortUrlWidget : Q_NULLPTR);
}
