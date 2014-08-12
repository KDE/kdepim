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
#include "sieveeditortabwidget.h"
#include "editor/sieveeditor.h"

#include <KLocalizedString>

#include <KSharedConfig>
#include <QTabWidget>
#include <KGlobalSettings>
#include <KColorScheme>
#include <KMessageBox>


#include <QSplitter>
#include <QTabBar>

SieveEditorMainWidget::SieveEditorMainWidget(QWidget *parent)
    : QSplitter(parent)
{
    mTabWidget = new SieveEditorTabWidget;
    connect(mTabWidget, &SieveEditorTabWidget::tabCloseRequestedIndex, this, &SieveEditorMainWidget::slotTabCloseRequested);
    connect(mTabWidget, &SieveEditorTabWidget::tabRemoveAllExclude, this, &SieveEditorMainWidget::slotTabRemoveAllExclude);
    addWidget(mTabWidget);
    mScriptManagerWidget = new SieveEditorScriptManagerWidget;
    connect(mScriptManagerWidget, SIGNAL(createScriptPage(QUrl,QStringList,bool)), this, SLOT(slotCreateScriptPage(QUrl,QStringList,bool)));
    connect(mScriptManagerWidget, SIGNAL(updateButtons(bool,bool,bool,bool)), SIGNAL(updateButtons(bool,bool,bool,bool)));
    connect(mScriptManagerWidget, &SieveEditorScriptManagerWidget::scriptDeleted, this, &SieveEditorMainWidget::slotScriptDeleted);
    connect(mScriptManagerWidget, SIGNAL(serverSieveFound(bool)), this, SIGNAL(serverSieveFound(bool)));
    connect(this, &SieveEditorMainWidget::updateScriptList, mScriptManagerWidget, &SieveEditorScriptManagerWidget::slotRefreshList);
    addWidget(mScriptManagerWidget);
    setChildrenCollapsible(false);
    QList<int> splitterSizes;
    splitterSizes << 80 << 20;
    KConfigGroup myGroup( KSharedConfig::openConfig(), "SieveEditorMainWidget" );
    setSizes(myGroup.readEntry( "mainSplitter", splitterSizes));
    connect(KGlobalSettings::self(), &KGlobalSettings::kdisplayPaletteChanged, this, &SieveEditorMainWidget::slotGeneralPaletteChanged);
}

SieveEditorMainWidget::~SieveEditorMainWidget()
{
    KConfigGroup myGroup( KSharedConfig::openConfig(), "SieveEditorMainWidget" );
    myGroup.writeEntry( "mainSplitter", sizes());
    myGroup.sync();
}

QWidget *SieveEditorMainWidget::hasExistingPage(const QUrl &url)
{
    for (int i=0; i < mTabWidget->count(); ++i) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(i));
        if (page) {
            if (page->currentUrl() == url) {
                return page;
            }
        }
    }
    return 0;
}

void SieveEditorMainWidget::slotScriptDeleted(const QUrl &url)
{
    QWidget *page = hasExistingPage(url);
    if (page) {
        mTabWidget->removeTab(mTabWidget->indexOf(page));
        delete page;
    }
}

void SieveEditorMainWidget::slotCreateScriptPage(const QUrl &url, const QStringList &capabilities, bool isNewScript)
{
    QWidget *page = hasExistingPage(url);
    if (page) {
        mTabWidget->setCurrentWidget(page);
    } else {
        SieveEditorPageWidget *editor = new SieveEditorPageWidget;
        connect(editor, SIGNAL(refreshList()), this, SIGNAL(updateScriptList()));
        connect(editor, SIGNAL(scriptModified(bool,SieveEditorPageWidget*)), this, SLOT(slotScriptModified(bool,SieveEditorPageWidget*)));
        connect(editor, SIGNAL(modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode)), SIGNAL(modeEditorChanged(KSieveUi::SieveEditorWidget::EditorMode)));
        editor->setIsNewScript(isNewScript);
        editor->loadScript(url, capabilities);
        mTabWidget->addTab(editor, url.fileName());
        mTabWidget->setCurrentWidget(editor);
        if (isNewScript)
            editor->saveScript(false, true);
    }
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

void SieveEditorMainWidget::editScript()
{
    mScriptManagerWidget->editScript();
}

void SieveEditorMainWidget::desactivateScript()
{
    mScriptManagerWidget->desactivateScript();
}

void SieveEditorMainWidget::refreshList()
{
    mScriptManagerWidget->refreshList();
}

void SieveEditorMainWidget::saveScript()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->saveScript();
        }
    }
}

bool SieveEditorMainWidget::needToSaveScript()
{
    bool scriptSaved = false;
    for (int i=0; i < mTabWidget->count(); ++i) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(i));
        if (page) {
            const bool result = page->needToSaveScript();
            if (result)
                scriptSaved = true;
        }
    }
    return scriptSaved;
}

QTabWidget *SieveEditorMainWidget::tabWidget() const
{
    return mTabWidget;
}

void SieveEditorMainWidget::slotGoToLine()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->goToLine();
        }
    }
}

void SieveEditorMainWidget::slotScriptModified(bool modified,SieveEditorPageWidget *page)
{
    const int index = mTabWidget->indexOf(page);
    if (index >= 0) {
        if (!mScriptColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        mTabWidget->tabBar()->setTabTextColor(index, modified ? mModifiedScriptColor : mScriptColor);
    }
}

void SieveEditorMainWidget::slotGeneralPaletteChanged()
{
    const QPalette pal = palette();
    mScriptColor = pal.text().color();
    mModifiedScriptColor = pal.text().color();

    const KColorScheme scheme( QPalette::Active, KColorScheme::View );
    mModifiedScriptColor = scheme.foreground( KColorScheme::NegativeText ).color();
}

void SieveEditorMainWidget::slotTabCloseRequested(int index)
{
    SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(index));
    if (page) {
        if (page->isModified()) {
            const int result = KMessageBox::questionYesNoCancel(this, i18n("Script was modified. Do you want to save before closing?"), i18n("Close script"));
            if (result == KMessageBox::Yes) {
                page->saveScript();
            } else if (result == KMessageBox::Cancel) {
                return;
            }
        }
        mTabWidget->removeTab(index);
        delete page;
    }
}

void SieveEditorMainWidget::slotTabRemoveAllExclude(int index)
{
    for(int i = mTabWidget->count()-1; i >=0; --i) {
        if (i == index) {
            continue;
        }
        slotTabCloseRequested(i);
    }
}

KSieveUi::SieveEditorWidget::EditorMode SieveEditorMainWidget::pageMode() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->pageMode();
        }
    }
    return KSieveUi::SieveEditorWidget::Unknown;
}
