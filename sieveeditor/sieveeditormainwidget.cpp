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

#include "sieveeditormainwidget.h"
#include "sieveeditorscriptmanagerwidget.h"
#include "sieveeditorpagewidget.h"
#include "editor/sieveeditor.h"

#include <KSharedConfig>

#include <QStackedWidget>
#include <QSplitter>

SieveEditorMainWidget::SieveEditorMainWidget(QWidget *parent)
    : QSplitter(parent)
{
    mStackedWidget = new QStackedWidget;
    addWidget(mStackedWidget);
    mScriptManagerWidget = new SieveEditorScriptManagerWidget;
    connect(mScriptManagerWidget, SIGNAL(createScriptPage(KUrl,QStringList,bool)), this, SLOT(slotCreateScriptPage(KUrl,QStringList,bool)));
    connect(mScriptManagerWidget, SIGNAL(updateButtons(bool,bool,bool,bool)), SIGNAL(updateButtons(bool,bool,bool,bool)));
    addWidget(mScriptManagerWidget);
    setChildrenCollapsible(false);
    QList<int> splitterSizes;
    splitterSizes << 80 << 20;
    KConfigGroup myGroup( KGlobal::config(), "SieveEditorMainWidget" );
    setSizes(myGroup.readEntry( "mainSplitter", splitterSizes));
}

SieveEditorMainWidget::~SieveEditorMainWidget()
{
    KConfigGroup myGroup( KGlobal::config(), "SieveEditorMainWidget" );
    myGroup.writeEntry( "mainSplitter", sizes());
    myGroup.sync();
}

void SieveEditorMainWidget::slotCreateScriptPage(const KUrl &url, const QStringList &capabilities, bool isNewScript)
{
    SieveEditorPageWidget *editor = new SieveEditorPageWidget;
    editor->setIsNewScript(isNewScript);
    editor->loadScript(url, capabilities);
    mStackedWidget->addWidget(editor);
    mStackedWidget->setCurrentWidget(editor);
}

void SieveEditorMainWidget::createNewScript()
{
    mScriptManagerWidget->slotCreateNewScript();
}

void SieveEditorMainWidget::deleteScript()
{
    mScriptManagerWidget->slotDeleteScript();
}

void SieveEditorMainWidget::updateServerList()
{
    mScriptManagerWidget->updateServerList();
}
