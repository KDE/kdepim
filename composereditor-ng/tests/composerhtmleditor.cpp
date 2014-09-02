/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "composerhtmleditor.h"
#include "widgets/domtreewidget.h"
#include "composerview.h"

#include <kapplication.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <KLocalizedString>
#include <qdebug.h>

#include <QSplitter>

ComposerHtmlEditor::ComposerHtmlEditor()
    : KXmlGuiWindow()
{
    QSplitter *w = new QSplitter;

    editor = new ComposerEditorNG::ComposerEditor(this);
    ComposerEditorNG::DomTreeWidget *domWidget = new ComposerEditorNG::DomTreeWidget(editor->view(), this);
    w->addWidget(domWidget);

    w->addWidget(editor);
    setCentralWidget(w);

    editor->createAllActions();
    editor->addCreatedActionsToActionCollection(actionCollection());
    QList<ComposerEditorNG::ComposerView::ComposerViewAction> lst;
    lst << ComposerEditorNG::ComposerView::Bold;
    editor->createToolBar(lst);
    setupActions();
    setupGUI();
}

ComposerHtmlEditor::~ComposerHtmlEditor()
{
}

void ComposerHtmlEditor::setupActions()
{
    KStandardAction::quit(kapp, SLOT(quit()),
                          actionCollection());
}

