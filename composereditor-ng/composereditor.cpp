/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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
#include "findreplacebar.h"
#include "composerview.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QWebFrame>

namespace ComposerEditorNG {


class ComposerEditorPrivate
{
public:
    ComposerEditorPrivate(ComposerEditor *qq)
        : findReplaceBar(0),
          q(qq),
          view(0),
          richTextEnabled(true)
    {
    }

    FindReplaceBar *findReplaceBar;
    ComposerEditor *q;
    ComposerView *view;

    bool richTextEnabled;
};
}


namespace ComposerEditorNG {


ComposerEditor::ComposerEditor(QWidget *parent)
    : QWidget(parent), d(new ComposerEditorPrivate(this))
{
    QVBoxLayout * vlay = new QVBoxLayout;
    vlay->setMargin(0);
    d->view = new ComposerView(this);
    vlay->addWidget(d->view);
    d->findReplaceBar = new FindReplaceBar(d->view);
    vlay->addWidget(d->findReplaceBar);
    setLayout(vlay);
    connect(d->view,SIGNAL(showFindBar()),d->findReplaceBar,SLOT(showAndFocus()));
    connect(d->view,SIGNAL(openLink(QUrl)),SIGNAL(openLink(QUrl)));
}

ComposerEditor::~ComposerEditor()
{
    QString content = d->view->page()->mainFrame()->toHtml();
    qDebug()<<"content "<<content;
    delete d;
}


void ComposerEditor::createActions(KActionCollection *actionCollection)
{
    d->view->createActions(actionCollection);
}


QString ComposerEditor::plainTextContent() const
{
    return d->view->page()->mainFrame()->toPlainText();
}

void ComposerEditor::setEnableRichText(bool richTextEnabled)
{
    if(d->richTextEnabled != richTextEnabled) {
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
}

#include "composereditor.moc"
