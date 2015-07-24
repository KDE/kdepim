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
#include "sievescriptdebuggertextedit.h"
#include <QAction>
#include <QMenu>

using namespace KSieveUi;

SieveScriptDebuggerTextEdit::SieveScriptDebuggerTextEdit(QWidget *parent)
    : KSieveUi::SieveTextEdit(parent)
{

}

SieveScriptDebuggerTextEdit::~SieveScriptDebuggerTextEdit()
{

}

void SieveScriptDebuggerTextEdit::addExtraMenuEntry(QMenu *menu, const QPoint &pos)
{
    if (!textCursor().hasSelection()) {
        //Add action to add debug.
    }
    KSieveUi::SieveTextEdit::addExtraMenuEntry(menu, pos);
}

