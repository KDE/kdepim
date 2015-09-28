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

#include "viewerplugintranslatorinterface.h"
#include "pimcommon/translatorwidget.h"

#include <QHBoxLayout>

#include <KActionCollection>

using namespace MessageViewer;

ViewerPluginTranslatorInterface::ViewerPluginTranslatorInterface(KActionCollection *ac, QWidget *parent)
    : ViewerPluginInterface(parent),
      mAction(Q_NULLPTR)
{
    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    mTranslatorWidget = new PimCommon::TranslatorWidget(this);
    layout->addWidget(mTranslatorWidget);
    createAction(ac);
}

ViewerPluginTranslatorInterface::~ViewerPluginTranslatorInterface()
{

}

void ViewerPluginTranslatorInterface::setText(const QString &text)
{
    mTranslatorWidget->setTextToTranslate(text);
}

QAction *ViewerPluginTranslatorInterface::action() const
{
    return mAction;
}

void ViewerPluginTranslatorInterface::createAction(KActionCollection *ac)
{

}
