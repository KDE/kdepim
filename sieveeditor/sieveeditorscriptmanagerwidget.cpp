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

SieveEditorScriptManagerWidget::SieveEditorScriptManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    setLayout(hbox);
    mTreeView = new SieveEditorManageSieveWidget;
    connect(mTreeView, SIGNAL(newScript(KUrl,QStringList)), this, SLOT(slotNewScript(KUrl,QStringList)));
    connect(mTreeView, SIGNAL(editScript(KUrl,QStringList)), this, SIGNAL(createNewScriptPage(KUrl,QStringList)));
    hbox->addWidget(mTreeView);
    mTreeView->slotRefresh();
}

SieveEditorScriptManagerWidget::~SieveEditorScriptManagerWidget()
{

}

void SieveEditorScriptManagerWidget::slotEditScript(const KUrl &url, const QStringList &capabilities)
{
    Q_EMIT createNewScriptPage(url, capabilities);
}

void SieveEditorScriptManagerWidget::slotNewScript(const KUrl &url, const QStringList &capabilities)
{
    //TODO
}

void SieveEditorScriptManagerWidget::addServerImap(const KUrl &url)
{
    if (!mUrls.contains(url))
        mUrls.append(url);
    //TODO
}

void SieveEditorScriptManagerWidget::slotCreateNewScript()
{
    mTreeView->slotNewScript();
}

void SieveEditorScriptManagerWidget::slotDeleteScript()
{
    mTreeView->slotEditScript();
}
