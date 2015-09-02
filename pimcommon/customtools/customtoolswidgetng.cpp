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

#include "customtoolswidgetng.h"
#include <QHBoxLayout>
#include <QStackedWidget>

using namespace PimCommon;

class PimCommon::CustomToolsWidgetNgPrivate
{
public:
    CustomToolsWidgetNgPrivate()
        : mStackedWidget(Q_NULLPTR)
    {

    }
    QStackedWidget *mStackedWidget;
};

CustomToolsWidgetNg::CustomToolsWidgetNg(QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::CustomToolsWidgetNgPrivate)
{
    QHBoxLayout *lay = new QHBoxLayout;
    d->mStackedWidget = new QStackedWidget;
    d->mStackedWidget->setObjectName(QStringLiteral("stackedwidget"));
    lay->addWidget(d->mStackedWidget);
    setLayout(lay);
}


CustomToolsWidgetNg::~CustomToolsWidgetNg()
{
    delete d;
}
