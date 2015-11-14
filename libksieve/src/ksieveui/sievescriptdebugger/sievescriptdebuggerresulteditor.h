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

#ifndef SIEVESCRIPTDEBUGGERRESULTEDITOR_H
#define SIEVESCRIPTDEBUGGERRESULTEDITOR_H

#include <kpimtextedit/plaintexteditor.h>

namespace KSieveUi
{
class SieveScriptDebuggerResultEditor : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT
public:
    explicit SieveScriptDebuggerResultEditor(QWidget *parent = Q_NULLPTR);
    ~SieveScriptDebuggerResultEditor();

protected:
    void addExtraMenuEntry(QMenu *menu, QPoint pos) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotClear();
    void slotSaveAs();
};
}
#endif // SIEVESCRIPTDEBUGGERRESULTEDITOR_H
