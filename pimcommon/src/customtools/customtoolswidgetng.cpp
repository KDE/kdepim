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

#include "customtoolsplugin.h"
#include "customtoolsviewinterface.h"
#include "customtoolswidgetng.h"
#include "customtools/customtoolspluginmanager.h"
#include <KToggleAction>

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QDebug>

using namespace PimCommon;

class PimCommon::CustomToolsWidgetNgPrivate
{
public:
    CustomToolsWidgetNgPrivate()
        : mStackedWidget(Q_NULLPTR)
    {

    }
    QStackedWidget *mStackedWidget;
    QList<PimCommon::CustomToolsViewInterface *> mListInterfaceView;
};

CustomToolsWidgetNg::CustomToolsWidgetNg(KActionCollection *ac, QWidget *parent)
    : QWidget(parent),
      d(new PimCommon::CustomToolsWidgetNgPrivate)
{
    QHBoxLayout *lay = new QHBoxLayout;
    d->mStackedWidget = new QStackedWidget;
    d->mStackedWidget->setObjectName(QStringLiteral("stackedwidget"));
    lay->addWidget(d->mStackedWidget);
    setLayout(lay);
    initializeView(ac);
    hide();
}

CustomToolsWidgetNg::~CustomToolsWidgetNg()
{
    delete d;
}

void CustomToolsWidgetNg::initializeView(KActionCollection *ac)
{
    QVector<CustomToolsPlugin *> localPluginsList = PimCommon::CustomToolsPluginManager::self()->pluginsList();
    Q_FOREACH (CustomToolsPlugin *plugin, localPluginsList) {
        PimCommon::CustomToolsViewInterface *localCreateView = plugin->createView(ac, this);
        d->mListInterfaceView.append(localCreateView);
        d->mStackedWidget->addWidget(localCreateView);
    }
}

void CustomToolsWidgetNg::slotToolsWasClosed()
{
    Q_FOREACH (PimCommon::CustomToolsViewInterface *interface, d->mListInterfaceView) {
        interface->action()->setChecked(false);
    }
    hide();
}

void CustomToolsWidgetNg::slotActivateView(QWidget *w)
{
    if (w) {
        d->mStackedWidget->setCurrentWidget(w);
        setVisible(true);
        Q_FOREACH (PimCommon::CustomToolsViewInterface *interface, d->mListInterfaceView) {
            if (interface != w) {
                interface->action()->setChecked(false);
            }
        }
        Q_EMIT toolActivated();
    } else {
        setVisible(false);
        slotToolsWasClosed();
    }
}

QList<KToggleAction *> CustomToolsWidgetNg::actionList() const
{
    QList<KToggleAction *> lstActions;
    lstActions.reserve(d->mListInterfaceView.count());
    Q_FOREACH (PimCommon::CustomToolsViewInterface *interface, d->mListInterfaceView) {
        lstActions << interface->action();
    }
    return lstActions;
}

void CustomToolsWidgetNg::setText(const QString &text)
{
    if (isVisible()) {
        Q_FOREACH (PimCommon::CustomToolsViewInterface *interface, d->mListInterfaceView) {
            if (interface == d->mStackedWidget->currentWidget()) {
                interface->setText(text);
                break;
            }
        }
    }
}
