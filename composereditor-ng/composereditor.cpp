/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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

#include "composereditor.h"
#include "composerview.h"
#include "widgets/findreplacebar.h"

#include <KToolBar>

#include <QDebug>
#include <QVBoxLayout>
#include <QWebFrame>

namespace ComposerEditorNG {


class ComposerEditorPrivate
{
public:
    ComposerEditorPrivate(ComposerEditor *qq)
        : findReplaceBar(0),
          toolbar(0),
          q(qq),
          view(0),
          richTextEnabled(true)
    {
    }

    void initialize()
    {
        QVBoxLayout * vlay = new QVBoxLayout;
        toolbar = new KToolBar(q);
        toolbar->setIconSize ( QSize ( 22, 22 ) );
        toolbar->setToolButtonStyle ( Qt::ToolButtonIconOnly );

        vlay->setMargin(0);
        vlay->addWidget(toolbar);
        toolbar->hide();
        vlay->addWidget(view);
        findReplaceBar = new FindReplaceBar(view);
        vlay->addWidget(findReplaceBar);
        q->setLayout(vlay);
        q->connect(view,SIGNAL(showFindBar()),findReplaceBar,SLOT(showAndFocus()));
        q->connect(view,SIGNAL(openLink(QUrl)),SIGNAL(openLink(QUrl)));
        q->connect(view->page(), SIGNAL(contentsChanged()), q, SIGNAL(textChanged()) );
    }

    FindReplaceBar *findReplaceBar;
    KToolBar *toolbar;
    ComposerEditor *q;
    ComposerView *view;

    bool richTextEnabled;
};

ComposerEditor::ComposerEditor(ComposerView *view, QWidget *parent)
    : QWidget(parent), d(new ComposerEditorPrivate(this))
{
    d->view = view;
    d->initialize();
}

ComposerEditor::ComposerEditor(QWidget *parent)
    : QWidget(parent), d(new ComposerEditorPrivate(this))
{
    d->view = new ComposerView(this);
    d->initialize();
}

ComposerEditor::~ComposerEditor()
{
    QString content = d->view->page()->mainFrame()->toHtml();
    qDebug()<<"content "<<content;
    delete d;
}

void ComposerEditor::addCreatedActionsToActionCollection(KActionCollection *actionCollection)
{
    d->view->addCreatedActionsToActionCollection(actionCollection);
}

QString ComposerEditor::plainTextContent() const
{
    return d->view->page()->mainFrame()->toPlainText();
}

void ComposerEditor::setEnableRichText(bool richTextEnabled)
{
    if (d->richTextEnabled != richTextEnabled) {
        d->richTextEnabled = richTextEnabled;
        d->view->setActionsEnabled(d->richTextEnabled);
    }
}

bool ComposerEditor::enableRichText() const
{
    return d->richTextEnabled;
}

bool ComposerEditor::isModified() const
{
    return d->view->page()->isModified();
}

void ComposerEditor::paste()
{
    d->view->page()->triggerAction(QWebPage::Paste);
}

void ComposerEditor::cut()
{
    d->view->page()->triggerAction(QWebPage::Cut);
}

void ComposerEditor::copy()
{
    d->view->page()->triggerAction(QWebPage::Copy);
}

void ComposerEditor::undo()
{
    d->view->page()->triggerAction(QWebPage::Undo);
}

void ComposerEditor::redo()
{
    d->view->page()->triggerAction(QWebPage::Redo);
}

QAction* ComposerEditor::action(QWebPage::WebAction action)
{
    return d->view->page()->action(action);
}

ComposerView *ComposerEditor::view() const
{
    return d->view;
}

void ComposerEditor::setHtmlContent( const QString& html )
{
    d->view->setHtmlContent(html);
}

QString ComposerEditor::htmlContent() const
{
    return d->view->page()->mainFrame()->toHtml();
}

void ComposerEditor::createActions(const QList<ComposerView::ComposerViewAction>& lstActions)
{
    d->view->createActions(lstActions);
}

void ComposerEditor::createAllActions()
{
    d->view->createAllActions();
}

void ComposerEditor::createToolBar(const QList<ComposerView::ComposerViewAction>& lstActions)
{
    if (d->toolbar) {
        d->view->createToolBar(lstActions,d->toolbar);
        d->toolbar->show();
    }
}

void ComposerEditor::addActionInToolBar(QAction *act)
{
    if (d->toolbar) {
        d->toolbar->addAction(act);
        d->toolbar->show();
    }
}

KToolBar *ComposerEditor::toolbar() const
{
    return d->toolbar;
}

}

#include "composereditor.moc"
