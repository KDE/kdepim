/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorscriptmanagerwidget.h"
#include "sieveeditormanagesievewidget.h"

#include <QHBoxLayout>
#include <QTreeWidget>

SieveEditorScriptManagerWidget::SieveEditorScriptManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
    mTreeView = new SieveEditorManageSieveWidget;
    connect(mTreeView, &SieveEditorManageSieveWidget::newScript, this, &SieveEditorScriptManagerWidget::slotNewScript);
    connect(mTreeView, &SieveEditorManageSieveWidget::editScript, this, &SieveEditorScriptManagerWidget::slotEditScript);
    connect(mTreeView, &SieveEditorManageSieveWidget::updateButtons, this, &SieveEditorScriptManagerWidget::slotUpdateButtons);
    connect(mTreeView, SIGNAL(scriptDeleted(QUrl)), this, SIGNAL(scriptDeleted(QUrl)));
    connect(mTreeView, SIGNAL(serverSieveFound(bool)), this, SIGNAL(serverSieveFound(bool)));
    hbox->addWidget(mTreeView);
}

SieveEditorScriptManagerWidget::~SieveEditorScriptManagerWidget()
{

}

void SieveEditorScriptManagerWidget::slotUpdateButtons(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    bool newScriptAction;
    bool editScriptAction;
    bool deleteScriptAction;
    bool desactivateScriptAction;
    mTreeView->enableDisableActions(newScriptAction, editScriptAction, deleteScriptAction, desactivateScriptAction);
    Q_EMIT updateButtons(newScriptAction, editScriptAction, deleteScriptAction, desactivateScriptAction);
}

void SieveEditorScriptManagerWidget::slotEditScript(const QUrl &url, const QStringList &capabilities)
{
    Q_EMIT createScriptPage(url, capabilities, false);
}

void SieveEditorScriptManagerWidget::slotNewScript(const QUrl &url, const QStringList &capabilities)
{
    Q_EMIT createScriptPage(url, capabilities, true);
}

void SieveEditorScriptManagerWidget::slotCreateNewScript()
{
    mTreeView->slotNewScript();
}

void SieveEditorScriptManagerWidget::slotDeleteScript()
{
    mTreeView->slotDeleteScript();
}

void SieveEditorScriptManagerWidget::slotRefreshList()
{
    updateServerList();
}

void SieveEditorScriptManagerWidget::updateServerList()
{
    mTreeView->slotRefresh();
}

void SieveEditorScriptManagerWidget::editScript()
{
    mTreeView->slotEditScript();
}

void SieveEditorScriptManagerWidget::desactivateScript()
{
    mTreeView->slotDeactivateScript();
}

void SieveEditorScriptManagerWidget::refreshList()
{
    mTreeView->slotRefresh();
}
