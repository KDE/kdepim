/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEEPLAINTEXTEDITOR_H
#define GRANTLEEPLAINTEXTEDITOR_H

#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"

namespace KPIMTextEdit
{
class TextEditorCompleter;
}
namespace KPIMTextEdit
{
class HtmlHighlighter;
}
namespace GrantleeThemeEditor
{
class GrantleePlainTextEditor : public PimCommon::PlainTextEditor
{
    Q_OBJECT
public:
    explicit GrantleePlainTextEditor(QWidget *parent = Q_NULLPTR);
    ~GrantleePlainTextEditor();

    void createCompleterList(const QStringList &extraCompletion);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

protected:
    KPIMTextEdit::TextEditorCompleter *mTextEditorCompleter;
private:
    void initCompleter();
    KPIMTextEdit::HtmlHighlighter *mHtmlHighlighter;
};
}

#endif // GRANTLEEPLAINTEXTEDITOR_H
