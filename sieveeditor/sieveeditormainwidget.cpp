/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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
#include "sieveeditoremptytabwidgetlabel.h"
#include "ksieveui/sieveeditor.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QTabWidget>
#include <KColorScheme>
#include <KMessageBox>

#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>

SieveEditorMainWidget::SieveEditorMainWidget(QWidget *parent)
    : QSplitter(parent)
{
    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QStringLiteral("stackedwidget"));

    mEditorEmptyLabel = new SieveEditorEmptyTabWidgetLabel;

    mTabWidget = new SieveEditorTabWidget;
    connect(mTabWidget, &SieveEditorTabWidget::tabCloseRequestedIndex, this, &SieveEditorMainWidget::slotTabCloseRequested);
    connect(mTabWidget, &SieveEditorTabWidget::tabRemoveAllExclude, this, &SieveEditorMainWidget::slotTabRemoveAllExclude);
    connect(mTabWidget, &SieveEditorTabWidget::tabCloseAllTab, this, &SieveEditorMainWidget::slotTabCloseAllRequested);

    mStackedWidget->addWidget(mTabWidget);
    mStackedWidget->addWidget(mEditorEmptyLabel);
    addWidget(mStackedWidget);

    mScriptManagerWidget = new SieveEditorScriptManagerWidget;
    connect(mScriptManagerWidget, &SieveEditorScriptManagerWidget::createScriptPage, this, &SieveEditorMainWidget::slotCreateScriptPage);
    connect(mScriptManagerWidget, &SieveEditorScriptManagerWidget::updateButtons, this, &SieveEditorMainWidget::updateButtons);
    connect(mScriptManagerWidget, &SieveEditorScriptManagerWidget::scriptDeleted, this, &SieveEditorMainWidget::slotScriptDeleted);
    connect(mScriptManagerWidget, &SieveEditorScriptManagerWidget::serverSieveFound, this, &SieveEditorMainWidget::serverSieveFound);
    connect(this, &SieveEditorMainWidget::updateScriptList, mScriptManagerWidget, &SieveEditorScriptManagerWidget::slotRefreshList);
    addWidget(mScriptManagerWidget);
    setChildrenCollapsible(false);
    QList<int> splitterSizes;
    splitterSizes << 80 << 20;
    KConfigGroup myGroup(KSharedConfig::openConfig(), "SieveEditorMainWidget");
    setSizes(myGroup.readEntry("mainSplitter", splitterSizes));
    updateStackedWidget();
}

SieveEditorMainWidget::~SieveEditorMainWidget()
{
    KConfigGroup myGroup(KSharedConfig::openConfig(), "SieveEditorMainWidget");
    myGroup.writeEntry("mainSplitter", sizes());
    myGroup.sync();
}

QWidget *SieveEditorMainWidget::hasExistingPage(const QUrl &url)
{
    for (int i = 0; i < mTabWidget->count(); ++i) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(i));
        if (page) {
            if (page->currentUrl() == url) {
                return page;
            }
        }
    }
    return Q_NULLPTR;
}

void SieveEditorMainWidget::updateStackedWidget()
{
    if (mTabWidget->count() == 0) {
        mStackedWidget->setCurrentWidget(mEditorEmptyLabel);
    } else {
        mStackedWidget->setCurrentWidget(mTabWidget);
    }
}

void SieveEditorMainWidget::slotScriptDeleted(const QUrl &url)
{
    QWidget *page = hasExistingPage(url);
    if (page) {
        mTabWidget->removeTab(mTabWidget->indexOf(page));
        delete page;
    }
    updateStackedWidget();
}

void SieveEditorMainWidget::slotCreateScriptPage(const QUrl &url, const QStringList &capabilities, bool isNewScript)
{
    QWidget *page = hasExistingPage(url);
    if (page) {
        mTabWidget->setCurrentWidget(page);
    } else {
        SieveEditorPageWidget *editor = new SieveEditorPageWidget;
        connect(editor, &SieveEditorPageWidget::refreshList, this, &SieveEditorMainWidget::updateScriptList);
        connect(editor, &SieveEditorPageWidget::scriptModified, this, &SieveEditorMainWidget::slotScriptModified);
        connect(editor, &SieveEditorPageWidget::modeEditorChanged, this, &SieveEditorMainWidget::modeEditorChanged);
        connect(editor, &SieveEditorPageWidget::undoAvailable, this, &SieveEditorMainWidget::undoAvailable);
        connect(editor, &SieveEditorPageWidget::redoAvailable, this, &SieveEditorMainWidget::redoAvailable);
        connect(editor, &SieveEditorPageWidget::copyAvailable, this, &SieveEditorMainWidget::copyAvailable);
        editor->setIsNewScript(isNewScript);
        editor->loadScript(url, capabilities);
        mTabWidget->addTab(editor, url.fileName());
        mTabWidget->setCurrentWidget(editor);
        if (isNewScript) {
            editor->uploadScript(false, true);
        }
        updateStackedWidget();
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

void SieveEditorMainWidget::uploadScript()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->uploadScript();
        }
    }
}

bool SieveEditorMainWidget::needToSaveScript()
{
    bool scriptSaved = false;
    for (int i = 0; i < mTabWidget->count(); ++i) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(i));
        if (page) {
            const bool result = page->needToSaveScript();
            if (result) {
                scriptSaved = true;
            }
        }
    }
    return scriptSaved;
}

QTabWidget *SieveEditorMainWidget::tabWidget() const
{
    return mTabWidget;
}

bool SieveEditorMainWidget::isUndoAvailable() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->isUndoAvailable();
        }
    }
    return false;
}

bool SieveEditorMainWidget::isRedoAvailable() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->isRedoAvailable();
        }
    }
    return false;
}

bool SieveEditorMainWidget::isWordWrap() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->isWordWrap();
        }
    }
    return false;
}

bool SieveEditorMainWidget::hasSelection() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->hasSelection();
        }
    }
    return false;
}

void SieveEditorMainWidget::slotSelectAll()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->selectAll();
        }
    }
}

void SieveEditorMainWidget::slotCopy()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->copy();
        }
    }
}

void SieveEditorMainWidget::slotPaste()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->paste();
        }
    }
}

void SieveEditorMainWidget::slotCut()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->cut();
        }
    }
}

void SieveEditorMainWidget::slotUndo()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->undo();
        }
    }
}

void SieveEditorMainWidget::slotRedo()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->redo();
        }
    }
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

void SieveEditorMainWidget::slotFind()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->find();
        }
    }
}

void SieveEditorMainWidget::slotReplace()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->replace();
        }
    }
}

void SieveEditorMainWidget::slotShare()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->share();
        }
    }
}

void SieveEditorMainWidget::slotAutoGenerateScript()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->autoGenerateScript();
        }
    }
}

void SieveEditorMainWidget::slotCheckSyntax()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->checkSyntax();
        }
    }
}

void SieveEditorMainWidget::slotComment()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->comment();
        }
    }
}

void SieveEditorMainWidget::slotUncomment()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->uncomment();
        }
    }
}

void SieveEditorMainWidget::slotImport()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->import();
        }
    }
}

void SieveEditorMainWidget::slotCheckSpelling()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->checkSpelling();
        }
    }
}

void SieveEditorMainWidget::slotSaveAs()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->saveAs();
        }
    }
}

void SieveEditorMainWidget::slotReverseCase()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->reverseCase();
        }
    }
}

void SieveEditorMainWidget::slotDebugSieveScript()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->debugSieveScript();
        }
    }
}

void SieveEditorMainWidget::slotUpperCase()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->upperCase();
        }
    }
}

void SieveEditorMainWidget::slotSentenceCase()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->sentenceCase();
        }
    }
}

void SieveEditorMainWidget::slotLowerCase()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->lowerCase();
        }
    }
}

void SieveEditorMainWidget::slotScriptModified(bool modified, SieveEditorPageWidget *page)
{
    const int index = mTabWidget->indexOf(page);
    if (index >= 0) {
        if (!mScriptColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        mTabWidget->tabBar()->setTabTextColor(index, modified ? mModifiedScriptColor : mScriptColor);
    }
}

void SieveEditorMainWidget::slotZoomIn()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->zoomIn();
        }
    }
}

void SieveEditorMainWidget::slotZoomOut()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->zoomOut();
        }
    }
}

void SieveEditorMainWidget::slotZoomReset()
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->zoomReset();
        }
    }
}

void SieveEditorMainWidget::slotWordWrap(bool state)
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->wordWrap(state);
        }
    }
}

void SieveEditorMainWidget::slotGeneralPaletteChanged()
{
    const QPalette pal = palette();
    mScriptColor = pal.text().color();
    mModifiedScriptColor = pal.text().color();

    const KColorScheme scheme(QPalette::Active, KColorScheme::View);
    mModifiedScriptColor = scheme.foreground(KColorScheme::NegativeText).color();
}

void SieveEditorMainWidget::slotTabCloseRequested(int index)
{
    SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(mTabWidget->widget(index));
    if (page) {
        if (page->isModified()) {
            const int result = KMessageBox::questionYesNoCancel(this, i18n("Script was modified. Do you want to save before closing?"), i18n("Close script"));
            if (result == KMessageBox::Yes) {
                page->uploadScript();
            } else if (result == KMessageBox::Cancel) {
                return;
            }
        }
        mTabWidget->removeTab(index);
        delete page;
    }
    updateStackedWidget();
}

void SieveEditorMainWidget::slotTabCloseAllRequested()
{
    for (int i = mTabWidget->count() - 1; i >= 0; --i) {
        slotTabCloseRequested(i);
    }
}

void SieveEditorMainWidget::slotTabRemoveAllExclude(int index)
{
    for (int i = mTabWidget->count() - 1; i >= 0; --i) {
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

void SieveEditorMainWidget::openBookmarkUrl(const QUrl &url)
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            page->openBookmarkUrl(url);
        }
    }
}

QString SieveEditorMainWidget::currentHelpTitle() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->currentHelpTitle();
        }
    }
    return QString();
}

QUrl SieveEditorMainWidget::currentHelpUrl() const
{
    QWidget *w = mTabWidget->currentWidget();
    if (w) {
        SieveEditorPageWidget *page = qobject_cast<SieveEditorPageWidget *>(w);
        if (page) {
            return page->currentHelpUrl();
        }
    }
    return QUrl();
}
