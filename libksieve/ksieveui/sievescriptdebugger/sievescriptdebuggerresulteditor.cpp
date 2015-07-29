/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sievescriptdebuggerresulteditor.h"
#include <KLocalizedString>
#include <QAction>
#include <QMenu>

using namespace KSieveUi;

SieveScriptDebuggerResultEditor::SieveScriptDebuggerResultEditor(QWidget *parent)
    : PimCommon::PlainTextEditor(parent)
{

}

SieveScriptDebuggerResultEditor::~SieveScriptDebuggerResultEditor()
{

}

void SieveScriptDebuggerResultEditor::addExtraMenuEntry(QMenu *menu, const QPoint &pos)
{
    PimCommon::PlainTextEditor::addExtraMenuEntry(menu, pos);
    if (isReadOnly() && !document()->isEmpty()) {
        QAction *clearAction = new QAction(i18n("Clear"), menu);
        connect(clearAction, &QAction::triggered, this, &SieveScriptDebuggerResultEditor::slotClear);
        menu->addAction(clearAction);
    }
}

void SieveScriptDebuggerResultEditor::slotClear()
{
    clear();
}
