/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

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

#include "composerwebenginewidget.h"
#include "composerwebengine.h"
#include "widgets/findreplacebar.h"

#include <KToolBar>
#include <QVBoxLayout>

using namespace ComposerEditorWebEngine;

class ComposerEditorWebEngine::ComposerWebEngineWidgetPrivate
{
public:
    ComposerWebEngineWidgetPrivate(ComposerWebEngineWidget *qq)
        : toolBarLayout(Q_NULLPTR),
          webEngine(Q_NULLPTR),
          q(qq),
          richTextEnabled(true)
    {

    }
    void initialize();
    KToolBar *createToolBar(const QList<ComposerWebEngine::ComposerWebEngineAction> &lstActions);

    void showToolBars(bool visible);

    QList<KToolBar *> listToolBar;
    QVBoxLayout *toolBarLayout;

    FindReplaceBar *findReplace;
    ComposerWebEngine *webEngine;
    ComposerWebEngineWidget *q;
    bool richTextEnabled;
};


void ComposerWebEngineWidgetPrivate::initialize()
{
    QVBoxLayout *vbox = new QVBoxLayout(q);

    toolBarLayout = new QVBoxLayout;
    toolBarLayout->setMargin(0);
    vbox->addLayout(toolBarLayout);
    vbox->setMargin(0);

    if (!webEngine) {
        webEngine = new ComposerWebEngine(q);
    }
    webEngine->setObjectName(QStringLiteral("webengine"));
    vbox->addWidget(webEngine);
    findReplace = new FindReplaceBar(webEngine);
    findReplace->setObjectName(QStringLiteral("findbar"));
    vbox->addWidget(findReplace);
    q->connect(webEngine, &ComposerWebEngine::showFindBar, findReplace, &FindReplaceBar::showAndFocus);
#if 0
    q->connect(view, &ComposerView::openLink, q, &ComposerEditor::openLink);
    q->connect(view->page(), &QWebPage::contentsChanged, q, &ComposerEditor::textChanged);
#endif
}

KToolBar *ComposerWebEngineWidgetPrivate::createToolBar(const QList<ComposerWebEngine::ComposerWebEngineAction> &lstActions)
{
    KToolBar *toolbar = new KToolBar(q);
    toolbar->setIconSize(QSize(22, 22));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBarLayout->addWidget(toolbar);
    webEngine->createToolBar(lstActions, toolbar);
    listToolBar.append(toolbar);
    return toolbar;
}

void ComposerWebEngineWidgetPrivate::showToolBars(bool visible)
{
    Q_FOREACH (KToolBar *toolBar, listToolBar) {
        toolBar->setVisible(visible);
    }
}

ComposerWebEngineWidget::ComposerWebEngineWidget(QWidget *parent)
    : QWidget(parent),
      d(new ComposerEditorWebEngine::ComposerWebEngineWidgetPrivate(this))
{
    d->initialize();
}

ComposerWebEngineWidget::ComposerWebEngineWidget(ComposerWebEngine *view, QWidget *parent)
    : QWidget(parent), d(new ComposerEditorWebEngine::ComposerWebEngineWidgetPrivate(this))
{
    d->webEngine = view;
    d->initialize();
}

ComposerWebEngineWidget::~ComposerWebEngineWidget()
{
    delete d;
}

void ComposerWebEngineWidget::addCreatedActionsToActionCollection(KActionCollection *actionCollection)
{
    d->webEngine->addCreatedActionsToActionCollection(actionCollection);
}

QString ComposerWebEngineWidget::plainTextContent() const
{
    return {}; // FIXME d->webEngine->page()->mainFrame()->toPlainText();
}

void ComposerWebEngineWidget::setEnableRichText(bool richTextEnabled)
{
    if (d->richTextEnabled != richTextEnabled) {
        d->richTextEnabled = richTextEnabled;
        d->webEngine->setActionsEnabled(d->richTextEnabled);
        d->showToolBars(d->richTextEnabled);
    }
}

bool ComposerWebEngineWidget::enableRichText() const
{
    return d->richTextEnabled;
}

bool ComposerWebEngineWidget::isModified() const
{
    return true; //FIXME d->webEngine->page()->isModified();
}

void ComposerWebEngineWidget::paste()
{
    d->webEngine->page()->triggerAction(QWebEnginePage::Paste);
}

void ComposerWebEngineWidget::cut()
{
    d->webEngine->page()->triggerAction(QWebEnginePage::Cut);
}

void ComposerWebEngineWidget::copy()
{
    d->webEngine->page()->triggerAction(QWebEnginePage::Copy);
}

void ComposerWebEngineWidget::undo()
{
    d->webEngine->page()->triggerAction(QWebEnginePage::Undo);
}

void ComposerWebEngineWidget::redo()
{
    d->webEngine->page()->triggerAction(QWebEnginePage::Redo);
}

QAction *ComposerWebEngineWidget::action(ComposerWebEngine::ComposerWebEngineAction action) const
{
    return {};//TODO d->webEngine->page()->action(action);
}

ComposerWebEngine *ComposerWebEngineWidget::view() const
{
    return d->webEngine;
}

void ComposerWebEngineWidget::setHtmlContent(const QString &html)
{
    d->webEngine->setHtmlContent(html);
}

QString ComposerWebEngineWidget::htmlContent() const
{
    return {}; //TODO return d->webEngine->page()->mainFrame()->toHtml();
}

void ComposerWebEngineWidget::createActions(const QList<ComposerWebEngine::ComposerWebEngineAction> &lstActions)
{
    d->webEngine->createActions(lstActions);
}

void ComposerWebEngineWidget::createAllActions()
{
    d->webEngine->createAllActions();
}

KToolBar *ComposerWebEngineWidget::createToolBar(const QList<ComposerWebEngine::ComposerWebEngineAction> &lstActions)
{
    return d->createToolBar(lstActions);
}

void ComposerWebEngineWidget::addActionInToolBar(QAction *act, KToolBar *toolbar)
{
    if (toolbar) {
        toolbar->addAction(act);
    }
}

QList<KToolBar *> ComposerWebEngineWidget::toolbars() const
{
    return d->listToolBar;
}

QMap<QString, QString> ComposerWebEngineWidget::localImages() const
{
    return d->webEngine->localImages();
}
