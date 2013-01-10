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
#include "composerhtmleditor.h"
#include "utils/domtreewidget.h"
#include "composerview.h"

#include <kapplication.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kdebug.h>

#include <QVBoxLayout>
#include <QSplitter>

ComposerHtmlEditor::ComposerHtmlEditor()
    : KXmlGuiWindow()
{
    QSplitter *w = new QSplitter;

    editor = new ComposerEditorNG::ComposerEditor(this);
    ComposerEditorNG::DomTreeWidget *domWidget = new ComposerEditorNG::DomTreeWidget(editor->view(), this);
    w->addWidget(domWidget);

    w->addWidget( editor );
    setCentralWidget( w );


    editor->createActions( actionCollection() );
    setupActions();
    setupGUI();
}

ComposerHtmlEditor::~ComposerHtmlEditor()
{
}

void ComposerHtmlEditor::setupActions()
{
    KStandardAction::quit( kapp, SLOT(quit()),
                           actionCollection() );
}


#include "composerhtmleditor.moc"
